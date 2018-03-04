//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "espconn.h"
#include "esp82xxutil.h"
#include "commonservices.h"
#include "vars.h"
#include "gpio.h"
#include <mdns.h>

#include "x10.h"

#define procTaskPrio        0
#define procTaskQueueLen    1

#define X10_OUT(val) if (val) { X10_ON; }// else { X10_OFF; }
#define X10_ON PIN_OUT_SET = _BV(5)
#define X10_OFF PIN_OUT_CLEAR = _BV(5)
#define CMD_MASK 0b11111111111111

static volatile int inhibit = 0;
static volatile int ignore = 0;
static volatile os_timer_t some_timer;
static struct espconn *pUdpServer;
usr_conf_t * UsrCfg = (usr_conf_t*)(SETTINGS.UserData);
static volatile os_timer_t bitclock;

static uint8_t bits_remaining;
static uint32_t bits;
static uint8_t bit;
static uint8_t phases;

size_t queue_len;
uint16_t queue[32];
uint8_t do_phase_thing;

uint8_t rxbuf[1536];
size_t rxbuf_len;
size_t rxbuf_off;

//int ICACHE_FLASH_ATTR StartMDNS();


void on_zc_int(void*);
void user_rf_pre_init(void) { /*nothing*/ }
void post_command(uint16_t, uint16_t, uint16_t);
void bitclock_cb(void *v) {
  inhibit = 1;
  os_timer_disarm(&bitclock);

  if (do_phase_thing) {
    X10_ON;
  } else {
    X10_OFF;
  }

  if (phases) {
    os_timer_arm_us(&bitclock, do_phase_thing?800:1778, 0);
    do_phase_thing = !do_phase_thing;
    phases--;
  }

  ets_delay_us(100);
  inhibit = 0;
}

//Tasks that happen all the time.

os_event_t    procTaskQueue[procTaskQueueLen];

static void ICACHE_FLASH_ATTR procTask(os_event_t *events)
{
	CSTick( 0 );
	system_os_post(procTaskPrio, 0, 0 );
}

//Timer event.
static void ICACHE_FLASH_ATTR myTimer(void *arg)
{
  /*static int blah = 0;
  blah++;
  if (!bits_remaining && blah > 5) {
    post_command(HOUSE_A, UNIT_1, ON);
    blah = 0;
    }*/
	CSTick( 1 ); // Send a one to uart
}


//Called when new packet comes in.
static void ICACHE_FLASH_ATTR
udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *)arg;
        if (len > 1535) {
	  len = 1535;
	  printf("Warning: Packet too long: ");
	} else if (rxbuf_off < rxbuf_len) {
	  printf("Warning: Packet dropped; in progress: ");
	} else {
	  memcpy(rxbuf, pusrdata, len);
	  rxbuf[len] = '\0';
	  rxbuf_off = 0;
	  rxbuf_len = len;
	}

	pusrdata[len] = '\0';
	printf("%s\n", rxbuf);
}

void ICACHE_FLASH_ATTR charrx( uint8_t c )
{
	//Called from UART.
}

/*void handle_buf(char *buf) {
  uint32_t 
  while (buf) {
    if (buf == '0') {
      
    }
    buf++;
  }
  }*/

void post_command(uint16_t house, uint16_t unit, uint16_t cmd) {
  if (bits_remaining) {
    if (queue_len < 32) {
      //queue[queue_len++] = cmd << 9 | unit << 4 | house;
    }
  } else {
    printf("sending command\n");
    bits_remaining = 11;
    //                   09876543210
    uint16_t start =         0b0111;
    uint16_t house =     0b10010110;
    uint16_t unit =    0b1010010110;
    uint16_t command = 0b0110011010;

    // Charles says:
    // 1110011010010110100101010110011
    // 111011010010110100101
    
    bits = command << 12 /*| unit << 12*/ | house << 4 | start;
    bits_remaining = 22;
    ignore = 6;
    //next_cmd_mask = CMD_MASK;
    //next_cmd = cmd << 9 | unit << 4 | house;
  }
}

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	system_timer_reinit();

	uart0_sendStr("\r\nesp82XX Web-GUI\r\n" VERSSTR "\b");

//Uncomment this to force a system restore.
//	system_restore();

	CSSettingsLoad( 0 );
	CSPreInit();

    pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	ets_memset( pUdpServer, 0, sizeof( struct espconn ) );
	espconn_create( pUdpServer );
	pUdpServer->type = ESPCONN_UDP;
	pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	pUdpServer->proto.udp->local_port = COM_PORT;
	espconn_regist_recvcb(pUdpServer, udpserver_recv);

	if( espconn_create( pUdpServer ) )
	{
		while(1) { uart0_sendStr( "\r\nFAULT\r\n" ); }
	}

	CSInit();

	SetServiceName( "espcom" );
	AddMDNSName(    "esp82xx" );
	AddMDNSName(    "espcom" );
	AddMDNSService( "_http._tcp",    "An ESP82XX Webserver", WEB_PORT );
	AddMDNSService( "_espcom._udp",  "ESP82XX Comunication", COM_PORT );
	AddMDNSService( "_esp82xx._udp", "ESP82XX Backend",      BACKEND_PORT );

	//Add a process
	system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);

	//Timer example
	os_timer_disarm(&some_timer);
	os_timer_setfn(&some_timer, (os_timer_func_t *)myTimer, NULL);
	os_timer_arm(&some_timer, 100, 1);

	os_timer_disarm(&bitclock);
	os_timer_setfn(&bitclock, (os_timer_func_t *)bitclock_cb, NULL);

	printf( "Boot Ok.\n" );

	bits_remaining = 0;
	bits = 0;
	bit = 0;
	do_phase_thing = 0;

	PIN_DIR_INPUT = _BV(4);
	PIN_DIR_OUTPUT = _BV(5);

	// 1 = Yellow = ZC
	// 2 = Green  = Ground
	// 3 = Red    = RX Output
	// 4 = Black  = TX Input

	int i;
        ETS_GPIO_INTR_DISABLE(); // Disable gpio interrupts
        ETS_GPIO_INTR_ATTACH(on_zc_int, 0); // GPIO4 interrupt handler
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
        PIN_DIR_INPUT = _BV(4);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
	gpio_pin_intr_state_set(GPIO_ID_PIN(4), 3);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, _BV(4));
        ETS_GPIO_INTR_ENABLE(); // Disable gpio interrupts


	//wifi_set_sleep_type(LIGHT_SLEEP_T);
	//wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);

	system_os_post(procTaskPrio, 0, 0 );
}

void handle_bit() {
  if (rxbuf_off < rxbuf_len) {
    if (rxbuf[rxbuf_off++] == '1') {
      phases = 5;
      do_phase_thing = true;
      bitclock_cb(0);
    }
  }

  // ===================================
  /*if (next_cmd_mask) {
    // Grab the last bit off the next command
    uint8_t bit = next_cmd & 1;

    if (start_bits) {
      bit = (0b1110 >> start_bits) & 1;
    } else {
      if (complement) {
	next_cmd_mask >>= 1;
	next_cmd >>= 1;
	complement = false;
      } else {
	bit = ~bit & 1;
	complement = true;
      }
    }

    // Send on each phase
    if (bit) {
      phase = 6;

      
    }
    X10_OUT(bit);
    ets_delay_us(800);
    //ets_delay_us(800);
    X10_OFF;
      
    ets_delay_us(1778);
    //ets_delay_us(1778);
    }

    if (start_bits) {
      start_bits -= 1;
    }
  } else {
    if (queue_len > 0) {
      start_bits = 4;
      next_cmd_mask = CMD_MASK;
      next_cmd = queue[--queue_len];
    }
    }*/

}

// on zero crossing interrupt...
on_zc_int(void *a) {
  //static int i;

  uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

  if (ignore > 0) {
    ignore--;
    return;
  }

  if (!inhibit) {
    handle_bit();
    //os_timer_arm_us(&bitclock, 800, 0);
  }
  return;

  //X10_OUT(i);


  //i = !i;
  
  //printf("x");
}


//There is no code in this project that will cause reboots if interrupts are disabled.
void EnterCritical() {
  
}

void ExitCritical() { }


