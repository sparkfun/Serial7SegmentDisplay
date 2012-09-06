/* Serial 7 Segment Display Firmware
   Version: 3.0.1
   By: Jim Lindblom (SparkFun Electronics)
   Date: August 24, 2012
   License: This code is beerware: feel free to use it, with or without attribution,
   in your own projects. If you find it helpful, buy me a beer next time you see me
   at the local.
   
   Description: This firmware goes on the SparkFun Serial 7-Segment displays.
   !!! Add product page link
   You can send the display serial data over either UART, SPI, or I2C. It'll
   sequentially display what it reads. There are special commands to control 
   individual segments, clear the display, reset the cursor, adjust the display's 
   brightness, UART baud rate, i2c address or factory reset.
   
   Arduino addon: This code should remain with the Serial7Seg Arduino hardware
   addon. New pin defines are required for pins 22 and 23 - PB6:7. Because the
   Serial 7-Segment runs on the ATmega328's internal oscillator, these two pins
   open up for our use.

   Hardware: You can find the Serial 7-Segment Display Schematic here:
   !!! Add schematic link
   
*/
#include <Wire.h>  // Handles I2C
#include <EEPROM.h>  // Brightness, Baud rate, and I2C address are stored in EEPROM
#include "settings.h"  // Defines command bytes, EEPROM addresses, display data

/* Digit Defines 
   Used with cathodes[] array (below) to reference specific segments */
const int A_SEG = 0;
const int B_SEG = 1;
const int C_SEG = 2;
const int D_SEG = 3;
const int E_SEG = 4;
const int F_SEG = 5;
const int G_SEG = 6;
const int DP_SEG = 7;

/* Pin defines for LED anodes and cathodes */
const byte anodes[6] = {A2, A3, 3, 4, 2, 9};  // Dig0, Dig1, Dig2, Dig3, Colon, Apostrophe
const byte cathodes[8] = {8, A0, 6, A1, 23, 7, 5, 22};  // A, B, C, D, E, F, G, DP
//const byte cathodes[8] = {6, 8, A0, A1, 23, 7, 5, 22};  // A, B, C, D, E, F, G, DP

/* Struct for circular data buffer
   data received over UART, SPI and I2C are all sent into a single buffer */
struct dataBuffer
{
  unsigned char data[BUFFER_SIZE];  // THE data buffer
  unsigned int head;  // store new data at this index
  unsigned int tail;  // read oldest data from this index
} buffer;  // our data buffer is creatively named - buffer

unsigned char commandMode = 0;  // Used to indicate if a commandMode byte has been received

/* Struct for 4-digit, 7-segment display 
   Stores display value (digits),  decimal status (decimals) for each digit, 
   and cursor for overall display */
struct display
{
  unsigned char digits[4];
  unsigned char decimals;
  unsigned char cursor;
} display;  // displays be displays

/* displayPeriod conrols the brightness of the display 
   it controls how long in microseconds a display will be active  */
const int displayPeriodMax = 10000;  //microseconds - how long to run through one cycle of display PWM
int displayPeriod = 2000;  // microseconds, maximum of 3000 - length of time each digit on display is active

/* SPI byte received interrupt routine */
ISR(SPI_STC_vect)
{
  noInterrupts();  // don't be rude! I'll be quick...
  
  unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
  unsigned char c = SPDR;  // Read data byte into c, from SPI data register
  
  if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
  {
    buffer.data[buffer.head] = c;  // Store the data into the buffer's head
    buffer.head = i;  // update buffer head, since we stored new data
  }
  
  interrupts();  // Fine, you were saying?
}

/* UART0 byte received interrupt routine */
ISR(USART_RX_vect)
{
  noInterrupts();  // We'll be quick...
  
  unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
  unsigned char c = UDR0;  // Read data byte into c, from UART0 data register
  
  if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
  {
    buffer.data[buffer.head] = c;  // Store the data into the buffer's head
    buffer.head = i;  // update buffer head, since we stored new data
  }
  
  interrupts();  // Okay, resume interrupts
}

/* I2C byte receive interrupt routine
   Note: this isn't an ISR. I'm using wire library (because it just works), so
   Wire.onReceive(twiReceive); should be called */
void twiReceive(int rxCount)
{
  while(Wire.available() > 0)  // Do this while data is available in Wire buffer
  {
    unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
    unsigned char c = Wire.read();  // Read data byte into c, from Wire data buffer
    
    if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
    {
      buffer.data[buffer.head] = c;  // Store the data into the buffer's head
      buffer.head = i;  // update buffer head, since we stored new data
    }    
  }
}

/* The display data is updated on a Timer interrupt */
ISR(TIMER1_COMPA_vect)
{
  noInterrupts();
  
  // if head and tail are not equal, there's data to be read from the buffer
  if (buffer.head != buffer.tail)
    updateBufferData();  // updateBufferData() will update the display info, or peform special commands
    
  interrupts();
}

void setup()
{  
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  delayMicroseconds(1);
  pinMode(10, INPUT_PULLUP);
  
  /* Set the initial state of displays and decimals 'x' =  off */
  display.digits[0] = 'x';
  display.digits[1] = 'x';
  display.digits[2] = 'x';
  display.digits[3] = 'x';
  display.decimals = 0x00;  // Turn all decimals off
  
  display.cursor = 0;  // Set cursor to first (left-most) digit
  buffer.head = 0;  // Initialize buffer values
  buffer.tail = 0;  
  
  /* Initialize anode pin states to OUTPUTS, all LOWs */
  for (int i=0; i<6; i++)
  {
    pinMode(anodes[i], OUTPUT);
    digitalWrite(anodes[i], LOW);
  }
  
  /* Initialize cathode pin states to OUTPUTS all HIGHs */
  for (int i=0; i<8; i++)
  {
    pinMode(cathodes[i], OUTPUT);
    digitalWrite(cathodes[i], HIGH);
  }
  
  /* displayPeriod controls the brightness of our display
     read the brightness value from EEPROM and map the 0 */
  displayPeriod = map(EEPROM.read(BRIGHTNESS_ADDRESS), 0, 255, 0, 2000);
  
  setupTimer();  // Setup timer to control interval reading from buffer
  setupUart();   // initialize UART stuff (interrupts, enable, baud)
  setupSPI();    // Initialize SPI stuff (enable, mode, interrupts)
  setupTWI();    // Initialize I2C stuff (address, interrupt, enable)
    
  interrupts();  // Turn interrupts on, and les' go
}

/* The display is constantly PWM'd in the loop() */
void loop()
{
  int delayTimer = 0;
  
  for (int i=0; i<5; i++)  // Run through this once for each digit and once for the decimals
  {
    displayDigit(display.digits[i], i);  // Set all the segments correctly
    
    delayMicroseconds(displayPeriod+1);  // Blocking delay while the digit is on
  }
  
  clearDisplay();  // Clear the display, this is how we adjust brightness
  /* delay for whats left of our maximum displayPeriod */
  delayMicroseconds((displayPeriodMax - 5 * displayPeriod) + 1); 
}

/* updateBufferData(): This beast of a function is called by the Timer 1 ISR if there is
   new data in the buffer. 
   If the data controls display data, that'll be updated.
   If the data relates to a command, commandmode will be set accordingly or a command 
   will be executed from this function. */
void updateBufferData()
{
  /* First we read from the oldest data in the buffer  */
  unsigned char c = buffer.data[buffer.tail];
  buffer.tail = (buffer.tail + 1) % BUFFER_SIZE;  // and update the tail to the next oldest
  
  /* if the last byte received wasn't a command byte (commandMode=0)
     and if the data is displayable (0-0x76 or 0x78), the display will be updated*/
  if ((commandMode == 0) && ((c < 0x76) || (c == 0x78)))
  {
    display.digits[display.cursor] = c;  // just store the read data into the cursor-active digit
    display.cursor = ((display.cursor + 1) % 4);  // Increment cursor, set back to 0 if necessary
  }
  else if (c == RESET_CMD)  // If the received char is the reset command
  {
    for (int i=0; i<4; i++)
      display.digits[i] = 'x';  // clear all digits
    display.decimals = 0;  // clear all decimals
    display.cursor = 0;  // reset the cursor
  }
  else if (commandMode != 0)  // Otherwise, if data is non-displayable and we're in a commandMode
  {
    unsigned int baud = 103;  // Default to 9600 if non-usable data received
    
    switch (commandMode)
    {
      case DECIMAL_CMD:  // Decimal setting mode
        display.decimals = c;  // decimals are set by one byte
        break;
      case BRIGHTNESS_CMD:  // Brightness setting mode
        displayPeriod = map(c, 0, 255, 0, 2000);  // Adjust the amount of time digits are on
        EEPROM.write(BRIGHTNESS_ADDRESS, c);    // write the new value to EEPROM
        break;
      case BAUD_CMD:  // Baud setting mode 
        switch (c)
        {
          case 0: // 2400
            baud = 416;
            break;
          case 1:  // 4800
            baud = 207;
            break;
          case 2:  // 9600
            baud = 103;
            break;
          case 3:  // 14400
            baud = 68;
            break;
          case 4:  // 19200
            baud = 51;
            break;
          case 5:  // 38400
            baud = 25;
            break;
          case 6:  // 57600
            baud = 16;
            break;
          case 7:  // 76800
            baud = 12;
            break;
          case 8:  // 115200
            baud = 8;
            break;
          case 9:  // 250000
            baud = 3;
            break;
          case 10:  // 500000
            baud = 1;
            break;
          case 11:  // 1000000
            baud = 0;
            break;
        }
        UBRR0 = baud;  // UBRR0 is set with no regard to F_CPU, assuming 8MHz 2x speed
        EEPROM.write(BAUD_ADDRESS_H, (unsigned char)(baud>>8));  // Update EEPROM baud setting
        EEPROM.write(BAUD_ADDRESS_L, (unsigned char)(baud & 0xFF));
        break;
      case CURSOR_CMD:  // Set the cursor
        if (c <= 3)  // Limited error checking, if >3 cursor command will have no effect
          display.cursor = c;  // Update the cursor value
        break;
      case TWI_ADDRESS_CMD:  // Set the I2C Address
        if ((c > 0) && (c < 0x7F))  
        { // As long as the address value is valid msb can't be set, can't be 0 (general call)
          EEPROM.write(TWI_ADDRESS_ADDRESS, c);  // Update the EEPROM value
          Wire.begin(c);  // on-the fly I2C address update
        }
        break;
      case FACTORY_RESET_CMD:  // Factory reset
        factoryReset();  // Let's do that in a function
        break;
      case DIGIT1_CMD:  // Single-digit control for digit 1
        display.digits[0] = c | 0x80;  // set msb to indicate single digit control mode
        break;
      case DIGIT2_CMD:  // Single-digit control for digit 2
        display.digits[1] = c | 0x80;
        break;
      case DIGIT3_CMD:  // Single-digit control for digit 3
        display.digits[2] = c | 0x80;
        break;
      case DIGIT4_CMD:  // Single-digit control for digit 4
        display.digits[3] = c | 0x80;
        break;
    }
    /* Leaving commandMode 
      !!! If the commandMode isn't a valid command, we'll leave command mode, should be checked below? */
    commandMode = 0;
  }
  else  // Finally, if we weren't in command mode, if the byte isn't displayable, we'll enter command mode
  {
    commandMode = c;  // which command mode is reflected by value of commandMode
  }
}

/* seutpTimer(): Set up timer 1, which controls interval reading from the buffer */
void setupTimer()
{
  /* Timer 1 is se to CTC mode, 16-bit timer counts up to 0xFF */
  TCCR1B = (1<<WGM12) | (1<<CS10);
  OCR1A = 0x00FF;
  TIMSK1 = (1<<OCIE1A);  // Enable interrupt on compare
}

/* setupUart(): Initializes UART0 hardware pins, sets up UART interrupt 
   Sets baud rate, parity, stop bit and data bits */
void setupUart()
{
  pinMode(0, INPUT);  // RX set as an INPUT
  
  UCSR0A = (1<<U2X0);  // DOUBLE SPEEEEEEED!
  UCSR0B = (1<<RXCIE0) | (1<<RXEN0);   // Enable RX, RX complete interrupt
  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);  // Asynchronous, no parity, 1 stop bit, 8 bit
  
  //EEPROM.write(BAUD_ADDRESS_L, 0xFF);  // These are useful if you need to reset...
  //EEPROM.write(BAUD_ADDRESS_H, 0xFF);  // ...the baud rate values in EEPROM
  
  /* read the baud rate setting from EEPROM */
  unsigned int baud = (EEPROM.read(BAUD_ADDRESS_H)<<8) | EEPROM.read(BAUD_ADDRESS_L);
  
  /* Check if the baud rate setting is valid */
  if ((baud==416)||(baud==207)||(baud==103)||(baud==68)||(baud==51)||(baud==34)
      ||(baud==25)||(baud==16)||(baud==12)||(baud==8)||(baud==3)||(baud==1)||(baud==0))
  {
    UBRR0 = baud;  // and update the UBRR0 !!! This is dependent on double-speed and 8MHz clock
  }
  else  // if the baud rate setting was invalid, default to 9600
  {
    baud = 103;  // !!! dependent on 8MHz, double speed uart
    UBRR0 = baud;  // default to 9600
    EEPROM.write(BAUD_ADDRESS_L, baud);  // Update EEPROM to reflect 9600 buad
    EEPROM.write(BAUD_ADDRESS_H, 0);
  }
}

/* setupSPI(): Initialize SPI, sets up hardware pins and enables spi and receive interrupt
   SPI is set to MODE 0 (CPOL=0, CPHA=0), slave mode, LSB first */
void setupSPI()
{
  pinMode(13, INPUT);  // SCK
  pinMode(11, INPUT);  // MOSI
  pinMode(10, INPUT_PULLUP);  // SS
  
  SPCR = (1<<SPIE) | (1<<SPE);  // Enable SPI interrupt, enable SPI
  /* DORD = 0, LSB First
     MSTR = 0, SLAVE
     CPOL = 0, sck low when idle                  } MODE 0
     CPHA = 0, data sampled on leading clock edge } MODE 0
     SPR1:0 = 0, no effect (slave mode) */
}

/* setupTWI(): initializes I2C (err TWI! TWI! TWI!, can't bang that into my head enough)
   I'm using the rock-solid Wire library for this. We'll initialize TWI, setup the address,
   and tell it what interrupt function to jump to when data is received. */
void setupTWI()
{
  unsigned char twiAddress;
  
  twiAddress = EEPROM.read(TWI_ADDRESS_ADDRESS);  // read the TWI address from 
  
  if ((twiAddress == 0) || (twiAddress > 0x7F))  
  { // If the TWI address is invalid, use a default address
    twiAddress = TWI_ADDRESS_DEFAULT;
    EEPROM.write(TWI_ADDRESS_ADDRESS, TWI_ADDRESS_DEFAULT);
  }
  
  Wire.begin(twiAddress);  // Initialize Wire library as slave at twiAddress address
  Wire.onReceive(twiReceive);  // setup interrupt routine for when data is received
}

/* factoryReset(): Uhoh! If something breaks, try sending a factory reset command to the device
   This will reset baud, TWI address, and brightness to default values */
void factoryReset()
{
  /* Reset Baud (9600) */
  unsigned int baud = BAUD_DEFAULT;
  UBRR0 = baud;
  EEPROM.write(BAUD_ADDRESS_L, (unsigned char)(baud & 0xFF));
  EEPROM.write(BAUD_ADDRESS_H, (unsigned char)(baud >> 8));
  
  
  /* Reset TWI Address (0x71) */
  Wire.begin(TWI_ADDRESS_DEFAULT);
  EEPROM.write(TWI_ADDRESS_ADDRESS, TWI_ADDRESS_DEFAULT);
  
  /* Reset Brightness (FULL BRIGHNESS!) */
  displayPeriod = map(255, 0, 255, 0, 2000);
  EEPROM.write(BRIGHTNESS_ADDRESS, BRIGHTNESS_DEFAULT);
  
}

/* displayDigit(byte number, byte digit): Displays number on digit.
   This function actually displays stuff. It sets the anodes to turn activate a digit,
   and it sets the cathodes to turn on the proper segments. It'll decode 0-F and ASCII
   e.g. displayDigit(8, 0) makes the left-most digit display '8'.
   displayDigit('b', 3) displays 'b' on 3. 
   if digit=4, the colon and apostrophes are controlled */
void displayDigit(byte number, byte digit)
{
  clearDisplay();  // Clear the display
  digitalWrite(anodes[digit], HIGH);  // pull the proper anode HIGH
      
  if (digit == 4)  // if digit=4, the colon and apostrophe are being controlled
  {
    digitalWrite(anodes[5], HIGH);  // We'll also need to activate the apostrophe anode
    if ((display.decimals & (1<<4)))  // Turn the colon on if bit set
      digitalWrite(6, LOW);  // Colon cathode is shared with A segment cathode
    if ((display.decimals & (1<<5)))  // Turn the apostrophe on if bit set
      digitalWrite(7, LOW);  // Apostrophe cathode shared with F segment cathode
  }
  else  // otherwise digit should be 0-3
  {
    if (number & 0x80)  // If msb is set, we're in single-digit control mode for this digit
    {
      for (int i=0; i<7; i++)  // in single digit control mode ASCII isn't decode, bit-for-segment control
      {
        if (number & (1<<i))  // if a bit is set
          digitalWrite(cathodes[i], LOW);  // turn on the corresponding segment
      }
    }
    else  // otherwise, we need to decode the ASCII or value of number before it's displayed
    {
      for (int i=0; i<7; i++)
      {
        // displayArray (defined in settings.h) decides which segments are turned on for each value of number
        if (displayArray[number][i])
          digitalWrite(cathodes[i], LOW);  // if the bit is set, turn on that segment
      }
    }
    /* finally, if the decimal bit for this digit is set, turn on the DP */
    if ((display.decimals & (1<<digit)))
      digitalWrite(cathodes[DP_SEG], LOW);
  }
}

/* clearDisplay(): Turns off everything! */
void clearDisplay()
{
  for (int i=0; i<6; i++)
    digitalWrite(anodes[i], LOW);  // anodes LOW
    
  for (int i=0; i<8; i++)
  {
    digitalWrite(cathodes[i], HIGH);  // cathodes HIGH
  }
}
