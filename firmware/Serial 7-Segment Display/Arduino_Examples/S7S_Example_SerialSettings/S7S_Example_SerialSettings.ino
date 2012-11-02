/*
 9-23-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This example code shows how to control the baud rate and other settings on the S7S.
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 7 on Uno (software serial RX) to TX on Serial7Segment
 Pin 8 on Uno to RX on Serial7Segment
 VIN to PWR
 GND to GND
 
*/

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

void setup() {

  Serial.begin(9600);
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
  
  Serial.println("Press enter to set display to 57600bps");
  while(Serial.available() < 0) ;

  //See the wiki for other baud rates
  //https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-baud
  Serial7Segment.write(0x7F);  // Baud rate control command
  Serial7Segment.write(6);  // Let's go to 57600bps  

  Serial.println("Display should now be at 57600. Press enter to begin display demo at 57600bps");
  while(Serial.available() < 0) ;

  Serial7Segment.begin(57600); //Talk to the Serial7Segment at new 57600 bps
  Serial7Segment.write(0x7A);  // Brightness control command
  Serial7Segment.write((byte) 255);  // 0 is dimmest, 255 is brightest

  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
}

void loop() {
  Serial.print("Cycle: ");
  Serial.println(cycles++);
  
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", cycles); //Convert deciSecond into a string that is right adjusted

  Serial7Segment.print(tempString);

  delay(100);
}



