#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BRIGHT_ADDRESS 0
#define UART_ADDRESS 2

#define FOSC 8000000
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define	SEGMENT_PORT	PORTD
#define	DP_PORT			PORTC	
#define	DIGIT_PORT		PORTC	
#define DDR_SPI    PORTB
#define DD_MISO    PINB4

#define A	1
#define B	2
#define C	3
#define D	4
#define E	5
#define F	6
#define	G	7
#define	DP	5

#define DIGIT1	0
#define	DIGIT2	1
#define DIGIT3	2
#define	DIGIT4	3
#define	COL_A	4

#define	DIGIT_ON_TIME	5
#define BRIGHT_FACTOR	50	// Factor to increase the brightness (0-255) delay by

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

//Function Prototypes
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void display(uint8_t number, uint8_t digit);
void clearDisplay(void);
void check_Special(void);
void resetDisplay(void);
void ioinit(void);
void delay_ms(uint16_t x);
void delay_us(uint16_t x);
void writeEEPROM(char toWrite, uint8_t addr);
char readEEPROM(char addr);
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=