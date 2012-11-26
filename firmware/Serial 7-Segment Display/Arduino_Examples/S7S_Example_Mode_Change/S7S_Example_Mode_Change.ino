/*
 11-24-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This is example code puts the display into counter mode.
 
 Note: This code expects the display to be listening at 9600bps. If your display is not at 9600bps, you can
 do a software or hardware reset. See the Wiki for more info: 
 http://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-baud
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 8 on Uno (software serial TX) to RX on Serial7Segment
 VIN to PWR
 GND to GND
 Button on SDI (Increment)
 Button on SDO (Decrement)
 
*/

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

#define MODE_CMD  0x82

#define MODE_DATA     0
#define MODE_ANALOG   1
#define MODE_COUNTER  2

int cycles = 0;

void setup() {

  Serial.begin(9600);
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
  
  delay(10);
  Serial7Segment.write(MODE_CMD); //Change the mode of the display
//  Serial7Segment.write(MODE_COUNTER); //Enter counter mode. Any pulse on SDI will go up. Any pulse on SDO will go down.
//  Serial7Segment.write(MODE_ANALOG); //Enter analog mode. Unit will display the analog voltages detected on A6/A7.
  Serial7Segment.write((byte)MODE_DATA); //Return to normal mode. You can also do a hardware reset to return to normal data mode.
  
}

void loop() 
{
  //Do nothing. User must hit momentary buttons connected to SDI/SDO
}



