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
 
 Note: To use the additional pins, PB6 and PB7, on the ATmega328 we have to add 
 some maps to the pins_arduino.h file. This allows Arduino to identify PB6 as 
 digital pin 22, and PB7 as digital pin 23. Because the Serial 7-Segment runs on 
 the ATmega328's internal oscillator, these two pins open up for our use.
 
 */
#include <Wire.h>  // Handles I2C
#include <EEPROM.h>  // Brightness, Baud rate, and I2C address are stored in EEPROM
#include "settings.h"  // Defines command bytes, EEPROM addresses, display data
// This code uses the SevSeg library, which can be donwnloaded from:
// https://github.com/sparkfun/SevSeg
#include "SevSeg.h" //Library to control generic seven segment displays

SevSeg myDisplay; //Create an instance of the object

//This firmware works on three different hardware layouts
//Serial7Segment was the original and drives the segments directly from the ATmega
//OpenSegment uses PNP and NPN transistors to drive larger displays
//Serial7SegmentShield also drives the segments directly from the ATmega
#define S7S            1
#define OPENSEGMENT    2
#define S7SHIELD       3
#define DISPLAY_TYPE OPENSEGMENT

//Global variables
unsigned int analogValue6 = 0; //These are used in analog meter mode
unsigned int analogValue7 = 0;
unsigned char deviceMode; // This variable is useds to select which mode the device should be in
unsigned char commandMode = 0;  // Used to indicate if a commandMode byte has been received

// Struct for circular data buffer data received over UART, SPI and I2C are all sent into a single buffer
struct dataBuffer
{
  unsigned char data[BUFFER_SIZE];  // THE data buffer
  unsigned int head;  // store new data at this index
  unsigned int tail;  // read oldest data from this index
} 
buffer;  // our data buffer is creatively named - buffer

// Struct for 4-digit, 7-segment display 
// Stores display value (digits),  decimal status (decimals) for each digit, and cursor for overall display
struct display
{
  char digits[4];
  unsigned char decimals;
  unsigned char cursor;
} 
display;  // displays be displays

void setup()
{  
  setupDisplay(); //Initialize display stuff (common cathode, digits, brightness, etc)

  //We need to check emergency after we have initialized the display so that we can use the display during an emergency reset
  checkEmergencyReset(); //Look to see if the RX pin is being pulled low

  setupTimer();  // Setup timer to control interval reading from buffer
  setupUART();   // initialize UART stuff (interrupts, enable, baud)
  setupSPI();    // Initialize SPI stuff (enable, mode, interrupts)
  setupTWI();    // Initialize I2C stuff (address, interrupt, enable)
  setupAnalog(); // Initialize the analog inputs
  setupMode();   // Determine which mode we should be in

  interrupts();  // Turn interrupts on, and les' go

  //Preload the display buffer with a default
  display.digits[0] = 1;
  display.digits[1] = 2;
  display.digits[2] = 3;
  display.digits[3] = 4;
}

// The display is constantly PWM'd in the loop()
void loop()
{
  if(deviceMode == MODE_DATA)
  {
    displayData();
  }
  else if(deviceMode == MODE_COUNTER)
  {
    displayCounter();
  }
  else if(deviceMode == MODE_ANALOG)
  {
    displayAnalog();
  }

  //We will loop if we've received a new device mode command
}

//This is the normal mode where we display whatever data is coming in over UART, SPI, and I2C
void displayData()
{
  while(deviceMode == MODE_DATA)
  {
    //Just hang out and update the display as new data comes in
    myDisplay.DisplayString(display.digits, display.decimals); //(numberToDisplay, decimal point location)

    serialEvent(); //Check the serial buffer for new data
  }
}

//Turn off the SPI and watch for increment pulses on the SDO pin, decrement on SDI
void displayCounter()
{
  SPCR = 0; //Disable all SPI interrupts that may be turned on

  int counterIncrement = SPI_MOSI; //Labeled SDI
  int counterDecrement = SPI_MISO; //Labeled SDO

  pinMode(counterIncrement, INPUT_PULLUP);
  pinMode(counterDecrement, INPUT_PULLUP);

  int counter = 0; //Watches the overall count
  boolean incrementCounted = false; //Watches the toggle the counter pins
  boolean decrementCounted = false;

  while(deviceMode == MODE_COUNTER) //Loop until we receive a different mode command
  {
    //Check to see if there has been a low/high pulse on increment
    if(digitalRead(counterIncrement) == LOW)
    {
      delay(1); //Check the pin 1 ms later - this is for debounce
      myDisplay.DisplayString(display.digits, 0); //Update display so that it doesn't blink

      if(digitalRead(counterIncrement) == LOW)
      {
        if(incrementCounted == false) //Only increment counter if this is a new pulse
        {
          counter++;
          incrementCounted = true; //We have now counted this pulse
        }
      }
    }
    else
    {
      //The increment pin is high, so sdo can be counted again
      incrementCounted = false;
    }

    //Check to see if there has been a low/high pulse on increment
    if(digitalRead(counterDecrement) == LOW)
    {
      delay(1); //Check the pin 1 ms later - this is for debounce
      myDisplay.DisplayString(display.digits, 0); //Update display so that it doesn't blink
      
      if(digitalRead(counterDecrement) == LOW)
      {
        if(decrementCounted == false) //Only increment counter if this is a new pulse 
        {
          counter--;
          decrementCounted = true; //We have now counted this pulse
        }
      }
    }
    else
    {
      //The increment pin is high, so sdo can be counted again
      decrementCounted = false;
    }

    //Display this count
    //char tempString[10]; //Used for sprintf
    sprintf(display.digits, "%4d", counter); //Convert counter into a string that is right adjusted

    
    //int tempCounter = counter;
    // for(int x = 0 ; x < 4 ; x++)
    // {
    // display.digits[3 - x] = (tempCounter % 10); //Pull off the right most digit and store in display array
    // tempCounter /= 10; //Shave number down by one digit
    // }

    myDisplay.DisplayString(display.digits, 0); //(numberToDisplay, no decimals during counter mode)

    serialEvent(); //Check the serial buffer for new data
  }  
}

//Do nothing but analog reads
void displayAnalog()
{
  while(deviceMode == MODE_ANALOG)
  {
    analogValue6 = analogRead(A6);
    analogValue7 = analogRead(A7);

    //Serial.print("A6: ");
    //Serial.print(analogValue6);
    //Serial.print(" A7: ");
    //Serial.print(analogValue7);

    //Do calculation for 1st voltage meter
    float fvoltage6 = ((analogValue6 * 50) / (float)1024);
    int voltage6 = round(fvoltage6);
    display.digits[0] = voltage6 / 10;
    display.digits[1] = voltage6 % 10;

    //Do calculation for 2nd voltage meter
    float fvoltage7 = ((analogValue7 * 50) / (float)1024);
    int voltage7 = round(fvoltage7);
    display.digits[2] = voltage7 / 10;
    display.digits[3] = voltage7 % 10;

    display.decimals = ((1<<DECIMAL1) | (1<<DECIMAL3)); //Turn on the decimals next to digit1 and digit3
    myDisplay.DisplayString(display.digits, display.decimals); //(numberToDisplay, decimal point location)

    serialEvent(); //Check the serial buffer for new data
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
  else if ((c == RESET_CMD) && (!commandMode))  // If the received char is the reset command
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
      setupUART(); //Checks to see if this baud rate is valid and turns on UART at this speed
      break;
    case CURSOR_CMD:  // Set the cursor
      if (c <= 3)  // Limited error checking, if >3 cursor command will have no effect
        display.cursor = c;  // Update the cursor value
      break;
    case TWI_ADDRESS_CMD:  // Set the I2C Address
      EEPROM.write(TWI_ADDRESS_ADDRESS, c); // Update the EEPROM value
      setupTWI(); //Checks to see if I2C address is valid and begins I2C
      break;
    case MODE_CMD:  // Set the device mode (example: data, analog, counter)
      EEPROM.write(MODE_ADDRESS, c); // Update the EEPROM value
      setupMode(); //Checks to see if this mode is valid and then enters new mode
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

