/*
 9-23-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This is example code that shows how to display basic numbers on the display.
 
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

  Serial.begin(9600);
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps

  //Reset the display
  Serial7Segment.write('v'); //This forces the cursor to return to the beginning of the display
}

void loop() {
  Serial.print("Cycle: ");
  Serial.println(cycles++);
  
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", cycles); //Convert deciSecond into a string that is right adjusted
  //sprintf(tempString, "%d", cycles); //Convert deciSecond into a string that is left adjusted (requires digit 1 command)
  //sprintf(tempString, "%04d", cycles); //Convert deciSecond into a string with leading zeros
  //sprintf(tempString, "%4X", cycles); //Count in HEX, right adjusted
  //int negativeCycles = cycles * -1;
  //sprintf(tempString, "%4d", negativeCycles); //Shows a negative sign infront of right adjusted number

  //Note: This method works well as long as your number is less than or equal to 4 digits.
  //14422 will cause the display to wrap (5 digits)
  //-5766 will cause the display to wrap (5 digits)
  //To fix this, send a 'v' character or look at how to control the digit placement
  //https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Basic-Usage#wiki-cursor

  Serial7Segment.print(tempString);

//  delay(10);
}



