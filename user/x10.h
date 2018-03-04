#ifndef X10_H
#define X10_H

#define HIGH 0x1
#define LOW  0x0
#define BIT_DELAY 1778    	// 1778 us between bit repeats in a half-cycle
#define BIT_LENGTH 800		// each bit is slightly less than 1ms long

#define	HOUSE_A          0b0110
// ############################
#define HOUSE_B          0b1110
#define	HOUSE_C          0b0010
#define	HOUSE_D          0b1010
#define	HOUSE_E          0b0001
#define	HOUSE_F          0b1001
#define	HOUSE_G          0b0101
#define	HOUSE_H          0b1101
#define	HOUSE_I          0b0111
#define	HOUSE_J          0b1111
#define	HOUSE_K          0b0011
#define	HOUSE_L          0b1011
#define	HOUSE_M          0b0000
#define	HOUSE_N          0b1000
#define HOUSE_O          0b0100
#define HOUSE_P          0b1100

#define UNIT_1           0b00110
// #############################
#define UNIT_2           0b11100
#define UNIT_3           0b00100
#define UNIT_4           0b10100
#define UNIT_5           0b00010
#define UNIT_6           0b10010
#define UNIT_7           0b01010
#define UNIT_8           0b11010
#define UNIT_9           0b01110
#define UNIT_10          0b11110
#define UNIT_11          0b00110
#define UNIT_12          0b10110
#define UNIT_13          0b00000
#define UNIT_14          0b10000
#define UNIT_15          0b01000
#define UNIT_16          0b11000

#define ALL_UNITS_OFF    0b10000
#define ALL_LIGHTS_ON    0b11000
// #############################
#define ON               0b00101
#define OFF              0b00111
#define DIM              0b01001
#define BRIGHT           0b01011
#define ALL_LIGHTS_OFF   0b01101
#define EXTENDED_CODE    0b01111
#define HAIL_REQUEST     0b10001
#define HAIL_ACKNOWLEDGE 0b10011
#define PRE_SET_DIM      0b10101
#define EXTENDED_DATA    0b11001
#define STATUS_ON        0b11011
#define STATUS_OFF       0b11101
#define STATUS_REQUEST   0b11111

uint16_t house_codes[16] = {              // Lookup table for House Code
  HOUSE_A,
  HOUSE_B,
  HOUSE_C,
  HOUSE_D,
  HOUSE_E,
  HOUSE_F,
  HOUSE_G,
  HOUSE_H,
  HOUSE_I,
  HOUSE_J,
  HOUSE_K,
  HOUSE_L,
  HOUSE_M,
  HOUSE_N,
  HOUSE_O,
};

uint16_t unit_codes[16] = {
  UNIT_1,
  UNIT_2,
  UNIT_3,
  UNIT_4,
  UNIT_5,
  UNIT_6,
  UNIT_7,
  UNIT_8,
  UNIT_9,
  UNIT_10,
  UNIT_11,
  UNIT_12,
  UNIT_13,
  UNIT_14,
  UNIT_15,
  UNIT_16,
};

#endif
