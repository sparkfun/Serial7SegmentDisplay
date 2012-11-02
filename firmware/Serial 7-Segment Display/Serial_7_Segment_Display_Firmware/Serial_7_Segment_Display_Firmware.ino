/* Serial 7 Segment Display Firmware
 Version: 3.0.1
 By: Jim Lindblom (SparkFun Electronics)
 Date: August 24, 2012
 License: This code is beerware: feel free to use it, with or without attribution,
 in your own projects. If you find it helpful, buy me a beer next time you see me
 at the local pub.
 
 Description: This firmware goes on the SparkFun Serial 7-Segment displays.
 https://www.sparkfun.com/search/results?term=serial+7+segment&what=products
 
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
#include "SevSeg.h" //Library to control generic seven segment displays

SevSeg myDisplay; //Create an instance of the object

//Define global variables that will hold the user's settings from EEPROM
//unsigned int settingBrightness;
byte settingUARTSpeed;
byte settingTWIAddress;

// Struct for circular data buffer data received over UART, SPI and I2C are all sent into a single buffer
struct dataBuffer
{
  unsigned char data[BUFFER_SIZE];  // THE data buffer
  unsigned int head;  // store new data at this index
  unsigned int tail;  // read oldest data from this index
} 
buffer;  // our data buffer is creatively named - buffer

unsigned char commandMode = 0;  // Used to indicate if a commandMode byte has been received

// Struct for 4-digit, 7-segment display 
// Stores display value (digits),  decimal status (decimals) for each digit, and cursor for overall display
struct display
{
  char digits[4];
  unsigned char decimals;
  unsigned char cursor;
} 
display;  // displays be displays

// SPI byte received interrupt routine
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

// I2C byte receive interrupt routine
// Note: this isn't an ISR. I'm using wire library (because it just works), so
// Wire.onReceive(twiReceive); should be called
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

// The display data is updated on a Timer interrupt
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
  setupDisplay(); //Initialize display stuff (common cathode, digits, brightness, etc)

  //We need to check emergency after we have initialized the display so that we can use the display during an emergency reset
  checkEmergencyReset(); //Look to see if the RX pin is being pulled low

  setupTimer();  // Setup timer to control interval reading from buffer
  setupUART();   // initialize UART stuff (interrupts, enable, baud)
  setupSPI();    // Initialize SPI stuff (enable, mode, interrupts)
  setupTWI();    // Initialize I2C stuff (address, interrupt, enable)

  interrupts();  // Turn interrupts on, and les' go

  //Preload the display buffer, should only be used during code development
  display.digits[0] = 1;
  display.digits[1] = 2;
  display.digits[2] = 3;
  display.digits[3] = 4;

  myDisplay.SetBrightness(100); //Set the display to 100% bright
}

// The display is constantly PWM'd in the loop()
void loop()
{
  myDisplay.DisplayString(display.digits, display.decimals); //(numberToDisplay, decimal point location)
}

// This is effectively the UART0 byte received interrupt routine
//ISR(USART_RX_vect)
void serialEvent()
{
  while (Serial.available()) 
  {
    //noInterrupts();  // We'll be quick...

    unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
    //    unsigned char c = UDR0;  // Read data byte into c, from UART0 data register
    unsigned char c = Serial.read();  // Read data byte into c, from UART0 data register

    if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
    {
      buffer.data[buffer.head] = c;  // Store the data into the buffer's head
      buffer.head = i;  // update buffer head, since we stored new data
    }

    //interrupts();  // Okay, resume interrupts
  }
}

// updateBufferData(): This beast of a function is called by the Timer 1 ISR if there is new data in the buffer. 
// If the data controls display data, that'll be updated.
// If the data relates to a command, commandmode will be set accordingly or a command 
// will be executed from this function.
void updateBufferData()
{
  // First we read from the oldest data in the buffer
  unsigned char c = buffer.data[buffer.tail];
  buffer.tail = (buffer.tail + 1) % BUFFER_SIZE;  // and update the tail to the next oldest

  // if the last byte received wasn't a command byte (commandMode=0)
  // and if the data is displayable (0-0x76 or 0x78), the display will be updated
  if ((commandMode == 0) && ((c < 0x76) || (c == 0x78)))
  {
    display.digits[display.cursor] = c;  // just store the read data into the cursor-active digit
    display.cursor = ((display.cursor + 1) % 4);  // Increment cursor, set back to 0 if necessary
  }
  else if (c == RESET_CMD)  // If the received char is the reset command
  {
    for(int i = 0 ; i < 4 ; i++)
      display.digits[i] = 'x';  // clear all digits
    display.decimals = 0;  // clear all decimals
    display.cursor = 0;  // reset the cursor
  }
  else if (commandMode != 0)  // Otherwise, if data is non-displayable and we're in a commandMode
  {
    switch (commandMode)
    {
    case DECIMAL_CMD:  // Decimal setting mode
      display.decimals = c;  // decimals are set by one byte
      break;
    case BRIGHTNESS_CMD:  // Brightness setting mode
      EEPROM.write(BRIGHTNESS_ADDRESS, c);    // write the new value to EEPROM
      myDisplay.SetBrightness(c); //Set the display to this brightness level
      break;
    case BAUD_CMD:  // Baud setting mode 
      EEPROM.write(BAUD_ADDRESS, c);  // Update EEPROM with new baud setting
      setupUART(); //Checks to see if this baud rate is valis and turns on UART at this speed
      break;
    case CURSOR_CMD:  // Set the cursor
      if (c <= 3)  // Limited error checking, if >3 cursor command will have no effect
        display.cursor = c;  // Update the cursor value
      break;
    case TWI_ADDRESS_CMD:  // Set the I2C Address
      EEPROM.write(TWI_ADDRESS_ADDRESS, c); // Update the EEPROM value
      setupTWI(); //Checks to see if I2C address is valid an begins I2C
      break;
    case FACTORY_RESET_CMD:  // Factory reset
      setDefaultSettings();  // Reset baud, brightness, and TWI address
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
    // Leaving commandMode 
    // !!! If the commandMode isn't a valid command, we'll leave command mode, should be checked below?
    commandMode = 0;
  }
  else  // Finally, if we weren't in command mode, if the byte isn't displayable, we'll enter command mode
  {
    commandMode = c;  // which command mode is reflected by value of commandMode
  }
}

// seutpTimer(): Set up timer 1, which controls interval reading from the buffer
void setupTimer()
{
  // Timer 1 is se to CTC mode, 16-bit timer counts up to 0xFF
  TCCR1B = (1<<WGM12) | (1<<CS10);
  OCR1A = 0x00FF;
  TIMSK1 = (1<<OCIE1A);  // Enable interrupt on compare
}

//Sets up the hardware pins to control the 7 segments and display type
void setupDisplay()
{
  //Determine the display brightness
  byte settingBrightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  if(settingBrightness > BRIGHTNESS_DEFAULT) {
   settingBrightness = BRIGHTNESS_DEFAULT; //By default, unit will be brightest
   EEPROM.write(BRIGHTNESS_ADDRESS, settingBrightness);
   }
  myDisplay.SetBrightness(settingBrightness); //Set the display to 100% bright

  // Set the initial state of displays and decimals 'x' =  off
  display.digits[0] = 'x';
  display.digits[1] = 'x';
  display.digits[2] = 'x';
  display.digits[3] = 'x';
  display.decimals = 0x00;  // Turn all decimals off
  display.cursor = 0;  // Set cursor to first (left-most) digit

  buffer.head = 0;  // Initialize buffer values
  buffer.tail = 0;  

  //This pinout is for OpenSegment PCB layout
  //Declare what pins are connected to the digits
  int digit1 = 9; //Pin 12 on my 4 digit display
  int digit2 = 16; //Pin 9 on my 4 digit display
  int digit3 = 17; //Pin 8 on my 4 digit display
  int digit4 = 3; //Pin 6 on my 4 digit display

  //Declare what pins are connected to the segments
  int segA = 14; //Pin 11 on my 4 digit display
  int segB = 2; //Pin 7 on my 4 digit display
  int segC = 8; //Pin 4 on my 4 digit display
  int segD = 6; //Pin 2 on my 4 digit display
  int segE = 7; //Pin 1 on my 4 digit display
  int segF = 15; //Pin 10 on my 4 digit display
  int segG = 4; //Pin 5 on my 4 digit display
  int segDP= 5; //Pin 3 on my 4 digit display

  int numberOfDigits = 4; //Do you have a 2 or 4 digit display?

  int displayType = COMMON_CATHODE; //Your display is either common cathode or common anode

  //Initialize the SevSeg library with all the pins needed for this type of display
  myDisplay.Begin(displayType, numberOfDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);

}

//This sets up the UART with the stored baud rate in EEPROM
void setupUART()
{
  //Read what the current UART speed is from EEPROM memory
  //Default is 9600
  settingUARTSpeed = EEPROM.read(BAUD_ADDRESS);
  if(settingUARTSpeed > BAUD_1000000) //Check to see if the baud rate has ever been set
  {
    settingUARTSpeed = BAUD_9600; //Reset UART to 9600 if there is no baud rate stored
    EEPROM.write(BAUD_ADDRESS, settingUARTSpeed);
  }

  //Initialize the UART
  switch(settingUARTSpeed)
  {
    case(BAUD_2400):
    Serial.begin(2400);
    break;
    case(BAUD_4800):
    Serial.begin(4800);
    break;
    case(BAUD_9600):
    Serial.begin(9600);
    break;
    case(BAUD_19200):
    Serial.begin(19200);
    break;
    case(BAUD_38400):
    Serial.begin(38400);
    break;
    case(BAUD_57600):
    Serial.begin(57600);
    break;
    case(BAUD_115200):
    Serial.begin(115200);
    break;
    case(BAUD_250000):
    Serial.begin(250000);
    break;
    case(BAUD_500000):
    Serial.begin(500000);
    break;
    case(BAUD_1000000):
    Serial.begin(1000000);
    break;
  default:
    //We should never reach this state, but if we do
    Serial.begin(9600);    
    break;
  }

}
// setupSPI(): Initialize SPI, sets up hardware pins and enables spi and receive interrupt
// SPI is set to MODE 0 (CPOL=0, CPHA=0), slave mode, LSB first
void setupSPI()
{
  pinMode(SPI_SCK, INPUT);
  pinMode(SPI_MOSI, INPUT);
  pinMode(SPI_CS, INPUT_PULLUP);

  SPCR = (1<<SPIE) | (1<<SPE);  // Enable SPI interrupt, enable SPI
  // DORD = 0, LSB First
  // MSTR = 0, SLAVE
  // CPOL = 0, sck low when idle                  } MODE 0
  // CPHA = 0, data sampled on leading clock edge } MODE 0
  // SPR1:0 = 0, no effect (slave mode)
}

// setupTWI(): initializes I2C (err TWI! TWI! TWI!, can't bang that into my head enough)
// I'm using the rock-solid Wire library for this. We'll initialize TWI, setup the address,
// and tell it what interrupt function to jump to when data is received.
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

//Check to see if we need an emergency system reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then reset the system settings
void checkEmergencyReset(void)
{
  pinMode(0, INPUT); //Turn the RX pin into an input
  digitalWrite(0, HIGH); //Push a 1 onto RX pin to enable internal pull-up

  //Quick pin check
  if(digitalRead(0) == HIGH) return;
  
  myDisplay.SetBrightness(100); //Set display to 100% brightness during emergency reset so we can see it

  //Wait 2 seconds, displaying reset-ish things while we wait
  for(uint8_t i = 0 ; i < 10 ; i++)
  {
    constantDisplay("____", 200);
    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore

    constantDisplay("----", 200);
    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore
  }		

  //If we make it here, then RX pin stayed low the whole time
  setDefaultSettings(); //Reset baud rate, brightness setting and TWI address

  //Now sit in a loop indicating system is now at 9600bps
  while(digitalRead(0) == LOW)
  {
    constantDisplay("000-", 500);
    constantDisplay("00-0", 500);
    constantDisplay("0-00", 500);
    constantDisplay("-000", 500);
  }
}

//Given a string, displays it costantly for a given amount of time
void constantDisplay (char *theString, long amountOfTime)
{
  long startTime = millis();
  while( (millis() - startTime) < amountOfTime)
  {
    for(byte x = 0 ; x < 10 ; x++)
    {
      myDisplay.DisplayString(theString, 0); //(numberToDisplay, decimal point location)
    }
  }
}

// In case of emergency, resets all the system settings to safe values
// This will reset baud, TWI address, and brightness to default values
void setDefaultSettings(void)
{
  //Reset UART to 9600bps
  EEPROM.write(BAUD_ADDRESS, BAUD_DEFAULT);
  settingUARTSpeed = BAUD_DEFAULT;

  //Reset system brightness to the brightest level
  EEPROM.write(BRIGHTNESS_ADDRESS, BRIGHTNESS_DEFAULT);
  myDisplay.SetBrightness(BRIGHTNESS_DEFAULT);
  
  //Reset the I2C address to the default 0x71
  EEPROM.write(TWI_ADDRESS_ADDRESS, TWI_ADDRESS_DEFAULT);
}
