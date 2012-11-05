/*
 11-2-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This is example code that shows how to control the brightness level of the display over I2C.
 
 Note: This code expects the display to be listening at the default I2C address. If your display is not at 0x71, you can
 do a software or hardware reset. See the Wiki for more info: 
 http://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 A5 to SCL
 A4 to SDA
 VIN to PWR
 GND to GND

*/

#include <Wire.h>

#define DISPLAY_ADDRESS1 0x71 //This is the default address of the OpenSegment with both solder jumpers open

void setup() 
{
  Wire.begin(); //Join the bus as master

  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenSegment Example Code");

  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write('v');
  Wire.endTransmission();
}

void loop() 
{
  Serial.println("Low brightness"); //Just a debug statement
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write(0x7A); // Brightness control command
  Wire.write(0); // Set brightness level: 0% to 100%
  Wire.endTransmission();
  i2cSendString("b000"); //Send the four characters to the display
  delay(2000); //Hang out for a bit before we go to the next brightness level

  Serial.println("Mid brightness"); //Just a debug statement
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write(0x7A); // Brightness control command
  Wire.write(50); // Set brightness level: 0% to 100%
  Wire.endTransmission();
  i2cSendString("b050"); //Send the four characters to the display
  delay(2000); //Hang out for a bit before we go to the next brightness level

  Serial.println("High brightness"); //Just a debug statement
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write(0x7A); // Brightness control command
  Wire.write(100); // Set brightness level: 0% to 100%
  Wire.endTransmission();
  i2cSendString("b100"); //Send the four characters to the display
  delay(2000); //Hang out for a bit before we go to the next brightness level
}

//Given a string, i2cSendString chops up the string and sends out the first four characters over i2c
void i2cSendString(char *toSend)
{
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  for(byte x = 0 ; x < 4 ; x++)
    Wire.write(toSend[x]); //Send a character from the array out over I2C
  Wire.endTransmission(); //Stop I2C transmission
}
