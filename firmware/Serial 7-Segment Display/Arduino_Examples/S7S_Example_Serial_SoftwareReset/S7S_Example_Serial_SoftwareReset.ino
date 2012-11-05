/*
 9-23-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 Did you set your Serial7Segment display to a baud rate you don't know? This example sketch
 should get you fixed up! This example code sends the factory reset command at 12 different baud rates.
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 8 on Uno to RX on Serial7Segment
 VIN to PWR
 GND to GND
 
*/

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

int cycles = 0;

void setup() 
{
  Serial.begin(9600); //Setup the debug terminal at regular 9600bps
  Serial.println("Attempting to reset display to 9600bps...");

  //Step through each available serial baud rate and send the factory reset command
  int baudRates[12] = {2400, 4800, 9600, 14400, 19200, 38400, 
  57600, 7600, 115200, 250000, 500000, 1000000};
  for (int i = 0 ; i < 12 ; i++)
  {
    Serial7Segment.begin(baudRates[i]);  // Set new baud rate
    delay(10);  // Arduino needs a moment to setup serial
    Serial7Segment.write(0x81);  // Send factory reset command
  }
  
  Serial7Segment.begin(9600);
  delay(10);  // Arduino needs a moment to setup serial
  Serial7Segment.write(0x76);  // Clear the display
  Serial7Segment.write('t');
  Serial7Segment.write('e');
  Serial7Segment.write('s');
  Serial7Segment.write('t');

  Serial.println("Attempt is now done. If the display says 'test', it has been reset to 9600bps");
}

void loop() 
{
  //Do nothing
}



