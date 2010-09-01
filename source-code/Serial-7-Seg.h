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

//Declare global variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
char data0=0;	// Digit 0 data
char data1=0;	// Digit 1 data
char data2=0;	// Digit 2 data
char data3=0;	// Digit 3 data
int receiveCount = 0;	// Will count from 0 to 3
uint16_t brightDelay = 0;	// Read from EEPROM on startup
char DPStatus = 0x00;	// Decimal point status, each bit represents one DP
int uartMode = 0;
int spiMode = 1;
char prevData0=0;	// Previous Digit 0 data
char prevData1=0;	// Previous Digit 1 data
char prevData2=0;	// Previous Digit 2 data
char prevData3=0;	// Previous Digit 3 data
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Function Prototypes
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void display(uint8_t number, uint8_t digit);
void clearDisplay(void);
void check_Special(void);
void resetDisplay(void);
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Global Use Functions
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void ioinit(void);
void delay_ms(uint16_t x);
void delay_us(uint16_t x);
void writeEEProm(char toWrite, char address);
char readEEProm(char address);
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void ioinit(void)
{
	char junk;

	sei();	// Enable interrupts

	// ************************************************** //
	//             Data directions:
	//               1 - output
	//               0 - input
	// Port B: (XTAL2)(APOS-A)(SCK)(MISO)(MOSI)(CSN)(NC)(NC)
	//            i      o     i    o     i     i   -   -
	DDRB = 0b01010000;
	// Port C: (RESET)(NC)(DP)(COL-A)(DIG4)(DIG3)(DIG2)(DIG1)
	//            i    -   o     o     o     o     o     o
	DDRC = 0b00111111;
	// Port D: (G)(F)(E)(D)(C)(B)(A)(RX)
	//          o  o  o  o  o  o  o  i
	DDRD = 0b11111110;
	// ************************************************** //

	//Init Timer0 for delay_us
	//Set Prescaler to clk/8 : 1click = 1us. CS01=1 
	TCCR0B = (1<<CS01); 

	// intialize USART Baud rate: 9600
	// Will default to 9600 if bad value in EEPROM
	// enable rx and rx interrupt
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXCIE0)|(1<<RXEN0);

	// Blink CSN for Test
	DDRB = 0b00000100;
	sbi(PORTB, 2);
	delay_ms(100);
	cbi(PORTB, 2);

	/* Set MISO output, PB6 output, all others input */
	DDRB = 0b01010000;
	/* Enable SPI */
	SPCR = (1<<SPIE) | (1<<SPE);
	/* Clear SPDR */
	junk = SPDR;
}

// Long delays
void delay_ms(uint16_t x)
{
	for (; x > 0 ; x--)
	{
		delay_us(250);
		delay_us(250);
		delay_us(250);
		delay_us(250);
	}
}

// For short delays
void delay_us(uint16_t x)
{
	if (x != 0)
	{
		while(x > 256)
		{
			TIFR0 = (1<<TOV0); //Clear any interrupt flags on Timer2
			TCNT0 = 0; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click
			while( (TIFR0 & (1<<TOV0)) == 0);
			
			x -= 256;
		}
	
		TIFR0 = (1<<TOV0); //Clear any interrupt flags on Timer2
		TCNT0 = 256 - x; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click
		while( (TIFR0 & (1<<TOV0)) == 0);
	}
}

// Write toWrite to addr in EEPROM
void writeEEPROM(char toWrite, uint8_t addr)
{
	// Write toWrite value to EEPROM address addr
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
		;
	/* Set up address and Data Registers */
	EEAR = addr;
	EEDR = toWrite;	// Write data1 into EEPROM
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

// Read EEPROM address addr and return value in EEDR
char readEEPROM(uint8_t addr)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
		;
	/* Set up address register */
	EEAR = addr;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);

	return EEDR;
}