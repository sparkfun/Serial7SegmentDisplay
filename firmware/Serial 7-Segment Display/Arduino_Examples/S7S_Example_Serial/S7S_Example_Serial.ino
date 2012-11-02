/*
 9-23-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This example code shows how to control the brightness and other system settings on S7S.
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 7 on Uno (software serial RX) to TX on Serial7Segment
 Pin 8 on Uno to RX on Serial7Segment
 VIN to PWR
 GND to GND
 
*/

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

char tempString[10]; //Used for sprintf
byte brightnessLevel = 0;

void setup() 
{
  Serial.begin(9600);
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
}

void loop() 
{
  //Set the display to three different levels to show brightness example

  brightnessLevel = 0; //0% brightness
  Serial.print("brightnessLevel: ");
  Serial.println(brightnessLevel);
  Serial7Segment.write(0x7A);  // Brightness control command
  Serial7Segment.write((byte) brightnessLevel);  // 0 is dimmest, 255 is brightest
  sprintf(tempString, "b%03d", brightnessLevel); //Convert deciSecond into a string that is right adjusted
  Serial7Segment.print(tempString);
  delay(2000);

  brightnessLevel = 50; //50% brightness
  Serial.print("brightnessLevel: ");
  Serial.println(brightnessLevel);
  Serial7Segment.write(0x7A);  // Brightness control command
  Serial7Segment.write((byte) brightnessLevel);  // 0 is dimmest, 255 is brightest
  sprintf(tempString, "b%03d", brightnessLevel); //Convert deciSecond into a string that is right adjusted
  Serial7Segment.print(tempString);
  delay(2000);

  brightnessLevel = 100; //100% brightness
  Serial.print("brightnessLevel: ");
  Serial.println(brightnessLevel);
  Serial7Segment.write(0x7A);  // Brightness control command
  Serial7Segment.write((byte) brightnessLevel);  // 0 is dimmest, 255 is brightest
  sprintf(tempString, "b%03d", brightnessLevel); //Convert deciSecond into a string that is right adjusted
  Serial7Segment.print(tempString);
  delay(2000);
}
