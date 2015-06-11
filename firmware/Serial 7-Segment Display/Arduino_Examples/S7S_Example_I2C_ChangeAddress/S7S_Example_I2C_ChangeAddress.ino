/*
 OpenSegment is an 7-segment display with Serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: June 11th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 OpenSegment gives the user multiple interfaces (serial, I2C, and SPI) to control a four digit
 seven segment display.
 
 This example shows how to change the I2C or TWI address.
 
 Please Note: 0x71 is the 7-bit I2C address. If you are using a different language than Arduino you will probably
 need to add the Read/Write bit to the end of the address. This means the default read address for the OpenSegment
 is 0b.1110.0011 or 0xE3 and the write address is 0b.1110.0010 or 0xE2.
 For more information see https://learn.sparkfun.com/tutorials/i2c

 Note: This code expects the display to be listening at the default I2C address. If your display is not at 0x71, you can
 do a software or hardware reset. See the Wiki for more info: 
 http://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands
 
 To get this code to work, attached a Serial7Segment to an Arduino Uno using the following pins:
 SCL (OpenSegment) to A5 (Arduino)
 SDA to A4
 VIN to 5V
 GND to GND

 For this example pull up resistors are not needed on SDA and SCL. If you have other devices on the
 I2C bus then 4.7k pull up resistors are recommended.
 
 OpenSegment will work at 400kHz Fast I2C. Use the .setClock() call shown below to set the data rate 
 faster if needed.

*/

#include <Wire.h>

#define DISPLAY_ADDRESS1 0x71 //This is the default address of the OpenSegment
#define DISPLAY_ADDRESS_NEW 0x50 //This is the new I2C address we want to go to

int cycles = 0;

void setup() 
{
  Wire.begin(); //Join the bus as master

  //By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
  //Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz

  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenSegment Example Code");

  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write('v');
  Wire.endTransmission();

  //Send change address command
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write(0x80); //Send TWI address change command
  Wire.write(DISPLAY_ADDRESS_NEW); //New I2C address to use
  Wire.endTransmission(); //Stop I2C transmission

  Serial.println("I2C address changed to 0x50");
  
  //Now we talk at this new address

  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  Wire.beginTransmission(DISPLAY_ADDRESS_NEW);
  Wire.write('v');
  Wire.endTransmission();
}

void loop() 
{
  cycles++; //Counting cycles! Yay!
  Serial.print("Cycle: ");
  Serial.println(cycles);
  
  i2cSendValue(cycles); //Send the four characters to the display
  
  delay(1); //If we remove the slow debug statements, we need a very small delay to prevent flickering
}

//Given a number, i2cSendValue chops up an integer into four values and sends them out over I2C
void i2cSendValue(int tempCycles)
{
  Wire.beginTransmission(DISPLAY_ADDRESS_NEW); // transmit to device at new I2C address
  Wire.write(tempCycles / 1000); //Send the left most digit
  tempCycles %= 1000; //Now remove the left most digit from the number we want to display
  Wire.write(tempCycles / 100);
  tempCycles %= 100;
  Wire.write(tempCycles / 10);
  tempCycles %= 10;
  Wire.write(tempCycles); //Send the right most digit
  Wire.endTransmission(); //Stop I2C transmission
}
