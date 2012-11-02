/*
 9-23-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This example code shows how to control the baud rate and other settings on the S7S. Open a terminal window at 9600bps
 to start the program. Send a 
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 7 on Uno (software serial RX) to TX on Serial7Segment
 Pin 8 on Uno to RX on Serial7Segment
 VIN to PWR
 GND to GND
 
*/

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

int cycles = 0;

void setup() {

  Serial.begin(9600); //Setup the debug terminal at regular 9600bps
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
  
  Serial.println("Send any character to set display to 57600bps");
  while(Serial.available()) Serial.read(); //Remove and ignore any characters sitting in the in buffer
  while(Serial.available() == 0) ; //Wait for you to press a key

  //See the wiki for other baud rates
  //https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-baud
  Serial7Segment.write(0x7F);  // Baud rate control command
  Serial7Segment.write(6);  // Let's go to 57600bps  

  Serial.println("Display should now be at 57600. Send any character to begin display demo at 57600bps");
  while(Serial.available()) Serial.read(); //Remove and ignore any characters sitting in the in buffer
  while(Serial.available() == 0) ; //Wait for you to press a key

  Serial7Segment.begin(57600); //Talk to the Serial7Segment at new 57600 bps
  Serial7Segment.write(0x7A); // Brightness control command
  Serial7Segment.write(100); // 0% is dimmest, 100% is brightest

  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
}

void loop() {
  //Serial.print("Cycle: "); //These debug statements slow down the loop significantly
  //Serial.println(cycles);
  cycles++;
  
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", cycles); //Convert deciSecond into a string that is right adjusted

  Serial7Segment.print(tempString); //Send serial string out the soft serial port to the S7S

  delay(1); //We need the slightest delay to allow the display to update sensibly
}



