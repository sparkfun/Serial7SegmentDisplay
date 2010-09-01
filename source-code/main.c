/*
	Serial 7-Segment Display v2
    5/12/10
    Spark Fun Electronics
	by: Jim Lindblom
	
	License: Creative Commons Attribution Share-Alike 3.0
	http://creativecommons.org/licenses/by-sa/3.0
*/

#include "Serial-7-Seg.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// SPI interrupt, enter when SPIF is set
SIGNAL(SPI_STC_vect)
{
	// Set mode to SPI
	// if previously in UART mode, reset receiveCount
	spiMode = 1;
	if (uartMode)
	{
		uartMode = 0;
		receiveCount = 0;
	}

	switch(receiveCount)
	{
		case 0:
			prevData0 = data0;
			data0 = SPDR;
			receiveCount++;
			break;
		case 1:
			prevData1 = data1;
			data1 = SPDR;
			receiveCount++;
			break;
		case 2:
			prevData2 = data2;
			data2 = SPDR;
			receiveCount++;
			break;
		case 3:
			prevData3 = data3;
			data3 = SPDR;
			receiveCount = 0;
			break;
	}
	check_Special();
}

// UART interrupt, enter when receive a character over RX
SIGNAL(USART_RX_vect)
{
	// Set mode to UART
	// if previously in SPI mode, reset receiveCount
	uartMode = 1;
	if (spiMode)
	{
		spiMode = 0;
		receiveCount = 0;
	}

	switch(receiveCount)
	{
		case 0:
			prevData0 = data0;
			data0 = UDR0;
			receiveCount++;
			break;
		case 1:
			prevData1 = data1;
			data1 = UDR0;
			receiveCount++;
			break;
		case 2:
			prevData2 = data2;
			data2 = UDR0;
			receiveCount++;
			break;
		case 3:
			prevData3 = data3;
			data3 = UDR0;
			receiveCount = 0;
			break;
	}
	check_Special();
}

int main(void)
{
	uint8_t UBaud = 2;

	ioinit();
	DDRB = 0b01010000;

	/* Read brightness from EEPROM */
	brightDelay = readEEPROM(BRIGHT_ADDRESS);
	if (brightDelay == 0xFF)
		brightDelay = 0;

	/* Read UART value from EEPROM */
	UBaud = readEEPROM(UART_ADDRESS);

	switch(UBaud)
	{
		case 0:	// 2400
			UBRR0H = 207 >> 8;
    		UBRR0L = 207;
			break;
		case 1:	// 4800
			UBRR0H = 103 >> 8;
    		UBRR0L = 103;
			break;
		case 2:	// 9600
			UBRR0H = 51 >> 8;
    		UBRR0L = 51;
			break;
		case 3:	// 14400
			UBRR0H = 34 >> 8;
    		UBRR0L = 34;
			break;
		case 4:	// 19200
			UBRR0H = 25 >> 8;
    		UBRR0L = 25;
			break;
		case 5:	// 38400
			UBRR0H = 12 >> 8;
    		UBRR0L = 12;
			break;
		case 6:	// 57600
			UBRR0H = 8 >> 8;
    		UBRR0L = 8;
			break;
	}
	

	// Main loop: update display
	while(1)
	{
		// Display numbers
		display(data0, 1);
		delay_us(DIGIT_ON_TIME);
		display(data1, 2);
		delay_us(DIGIT_ON_TIME);
		display(data2, 3);
		delay_us(DIGIT_ON_TIME);
		display(data3, 4);
		delay_us(DIGIT_ON_TIME);
		display(0, 5);
		delay_us(DIGIT_ON_TIME);
		display(0, 6);
		delay_us(DIGIT_ON_TIME);
		clearDisplay();
		delay_us(brightDelay*BRIGHT_FACTOR);
	}
}

// Output number to digit 0,1,2, or 3, 4 to display dots
void display(uint8_t character, uint8_t digit)
{	
	clearDisplay();

	//  Turn on selected digit	
	switch (digit)
	{
		case 1:
			sbi(DIGIT_PORT, DIGIT1);
			break;
		case 2:
			sbi(DIGIT_PORT, DIGIT2);
			break;
		case 3:
			sbi(DIGIT_PORT, DIGIT3);
			break;	
		case 4:
			sbi(DIGIT_PORT, DIGIT4);
			break;
	}
	//	Control colon
	if (digit == 5)
	{
		if (DPStatus & (1<<4))
		{
			cbi(PORTD, 3);
			sbi(PORTC, 4);
		}
	}
	else if (digit == 6)	// Control apostrophe
	{
		if (DPStatus & (1<<5))
		{
			sbi(PORTB, 6);
			cbi(PORTD, 4);
		}
	}
	else
	{
		if ((character & 0x80)==0x80)
		{
			character ^= 0xFF;
			SEGMENT_PORT = character << 1;
		}
		else{
			//  Turn on necessary segments
			switch (character)
			{
				case 0:
				case '0':
				case 'O':
					SEGMENT_PORT = (1<<G);
					break;
				case 1:
				case '1':
				case 'l':
					SEGMENT_PORT = (1<<A)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
					break;
				case 2:
				case '2':
					SEGMENT_PORT = (1<<C)|(1<<F);
					break;
				case 3:
				case '3':
					SEGMENT_PORT = (1<<E)|(1<<F);
					break;
				case 4:
				case '4':
					SEGMENT_PORT = (1<<A)|(1<<D)|(1<<E);
					break;
				case 5:
				case '5':
				case 'S':
				case 's':
					SEGMENT_PORT = (1<<B)|(1<<E);
					break;
				case 6:
				case '6':
					SEGMENT_PORT = (1<<B);
					break;
				case 7:
				case '7':
					SEGMENT_PORT = (1<<D)|(1<<E)|(1<<F)|(1<<G);
					break;
				case 8:
				case '8':
					SEGMENT_PORT = 0x00;
					break;
				case 9:
				case '9':
				case 'g':
					SEGMENT_PORT = (1<<E);
					break;
				case 10:
				case 'A':
				case 'a':
					SEGMENT_PORT = (1<<D);
					break;
				case 11:
				case 'B':
				case 'b':
					SEGMENT_PORT = (1<<A)|(1<<B);
					break;
				case 12:
				case 'C':
					SEGMENT_PORT = (1<<B)|(1<<C)|(1<<G);
					break;
				case 'c':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C)|(1<<F);
					break;
				case 13:
				case 'D':
				case 'd':
					SEGMENT_PORT = (1<<A)|(1<<F);
					break;
				case 14:
				case 'E':
					SEGMENT_PORT = (1<<B)|(1<<C);
					break;
				case 'e':
					SEGMENT_PORT = (1<<C);
					break;
				case 15:
				case 'F':
				case 'f':
					SEGMENT_PORT = (1<<B)|(1<<C)|(1<<D);
					break;
				case 'G':
					SEGMENT_PORT = (1<<B)|(1<<G);
					break;
				case 'H':
					SEGMENT_PORT = (1<<A)|(1<<D);
					break;
				case 'h':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<D);
					break;
				case 'I':
				case 'i':
					SEGMENT_PORT = (1<<A)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
					break;
				case 'J':
				case 'j':
					SEGMENT_PORT = (1<<A)|(1<<F)|(1<<G);
					break;
				case 'L':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C)|(1<<G);
					break;
				case 'N':
				case 'n':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<D)|(1<<F);
					break;
				case 'o':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<F);
					break;
				case 'P':
				case 'p':
					SEGMENT_PORT = (1<<C)|(1<<D);
					break;
				case 'Q':
				case 'q':
					SEGMENT_PORT = (1<<D)|(1<<E);
					break;
				case 'R':
				case 'r':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<F);
					break;
				case 'T':
				case 't':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C);
					break;
				case 'U':
					SEGMENT_PORT = (1<<A)|(1<<G);
					break;
				case 'u':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<F)|(1<<G);
					break;
				case 'Y':
				case 'y':
					SEGMENT_PORT = (1<<A)|(1<<E);
					break;
				case '-':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F);
					break;
				case '"':
					SEGMENT_PORT = (1<<A)|(1<<C)|(1<<D)|(1<<E)|(1<<G);
					break;
				case 0x27:	// "'"
					SEGMENT_PORT = (1<<A)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
					break;
				case '_':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<G);
					break;
				case 'x':
					SEGMENT_PORT = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
					break;
			}
		}
	}

	
	// Turn on decimal points depending on DPStatus
	if ((DPStatus & (1<<0))&&(digit==1))
	{
		cbi(DP_PORT, DP);
	}
	if ((DPStatus & (1<<1))&&(digit==2))
	{
		cbi(DP_PORT, DP);
	}
	if ((DPStatus & (1<<2))&&(digit==3))
	{
		cbi(DP_PORT, DP);
	}
	if ((DPStatus & (1<<3))&&(digit==4))
	{
		cbi(DP_PORT, DP);
	}
}

void clearDisplay(void)
{
	// Turn all LEDs off:
	PORTB = 0b00000000;
	PORTC = 0b00100000;
	PORTD = 0b11111110;
}

void check_Special(void)
{
	switch (receiveCount)
	{
		case 2:
			prevData2 = data2;
			prevData3 = data3;
			break;	
	}
	
	// If sent special character v
	// Reset receive count
	if ((data0=='v')||(data1=='v')||(data2=='v')||(data3=='v'))
	{
		resetDisplay();
	}
	// If sent special character z
	// Update brightness
	if ((data0=='z')&&(receiveCount==2))
	{
		// Write bright_level into EEPROM
		writeEEPROM(data1, BRIGHT_ADDRESS);

		brightDelay = data1;
		
		receiveCount = 0;
		data0 = prevData0;
		data1 = prevData1;
		data2 = prevData2;
		data3 = prevData3;
	}
	// If sent special character w
	// Update DPStatus
	if ((data0=='w')&&(receiveCount==2))
	{
		DPStatus = data1;

		receiveCount = 0;
		data0 = prevData0;
		data1 = prevData1;
		data2 = prevData2;
		data3 = prevData3;
	}
	// If sent special character 0x7F
	// Update baud rate
	// Warning: numbers are static and depend on 8MHz clock
	if ((data0==0x7F)&&(receiveCount==2))
	{
		switch(data1)
		{
			case 0:	// 2400
				UBRR0H = 207 >> 8;
    			UBRR0L = 207;
				break;
			case 1:	// 4800
				UBRR0H = 103 >> 8;
    			UBRR0L = 103;
				break;
			case 2:	// 9600
				UBRR0H = 51 >> 8;
    			UBRR0L = 51;
				break;
			case 3:	// 14400
				UBRR0H = 34 >> 8;
    			UBRR0L = 34;
				break;
			case 4:	// 19200
				UBRR0H = 25 >> 8;
    			UBRR0L = 25;
				break;
			case 5:	// 38400
				UBRR0H = 12 >> 8;
    			UBRR0L = 12;
				break;
			case 6:	// 57600
				UBRR0H = 8 >> 8;
    			UBRR0L = 8;
				break;
		}

		writeEEPROM(data1, UART_ADDRESS);
	
		receiveCount = 0;
		data0 = prevData0;
		data1 = prevData1;
		data2 = prevData2;
		data3 = prevData3;
	}
	// If sent special character '{' (0x7B)
	// Digit 0 single segment control
	if ((data0=='{')&&(receiveCount==2))
	{
		data0 = data1;
		data0 |= 0x80;	// Set MSB
		data1 = prevData1;
		data2 = prevData2;
		data3 = prevData3;
		
		receiveCount = 0;
	}
	// If sent special character '|' (0x7C)
	// Digit 0 single segment control
	if ((data0=='|')&&(receiveCount==2))
	{
		data1 = data1;
		data1 |= 0x80;	// Set MSB
		data0 = prevData0;
		data2 = prevData2;
		data3 = prevData3;
		
		receiveCount = 0;
	}
	// If sent special character '}' (0x7D)
	// Digit 0 single segment control
	if ((data0=='}')&&(receiveCount==2))
	{
		data2 = data1;
		data2 |= 0x80;	// Set MSB
		data1 = prevData1;
		data0 = prevData0;
		data3 = prevData3;
		
		receiveCount = 0;
	}
	// If sent special character '~' (0x7E)
	// Digit 0 single segment control
	if ((data0=='~')&&(receiveCount==2))
	{
		data3 = data1;
		data3 |= 0x80;	// Set MSB
		data1 = prevData1;
		data2 = prevData2;
		data0 = prevData0;
		
		receiveCount = 0;
	}
}

// Clear display and reset receiveCount
void resetDisplay(void)
{
	receiveCount = 0;
	data0 = 'x';
	data1 = 'x';
	data2 = 'x';
	data3 = 'x';
}