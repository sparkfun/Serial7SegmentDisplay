/*
 11-2-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This is example code that shows how to send data over SPI to the display.
 
 For more information about the commands, be sure to visit:
 http://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands
 
 To get this code to work, attached an OpenSegment to an Arduino Uno using the following pins:
 Pin 10 on Uno (CS) to CS on OpenSegment
 Pin 11 to MOSI
 Pin 12 to MISO
 Pin 13 to SCK
 VIN to PWR
 GND to GND

*/

#include <SPI.h>

int csPin = 10; //You can use any IO pin but for this example we use 10

int cycles = 0;

void setup() 
{
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH); //By default, don't be selecting OpenSegment

  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenSegment Example Code");

  SPI.begin(); //Start the SPI hardware
  SPI.setClockDivider(SPI_CLOCK_DIV64); //Slow down the master a bit

  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  SPI.transfer('v'); //Reset command
}

void loop() 
{
  cycles++; //Counting cycles! Yay!
  Serial.print("Cycle: ");
  Serial.println(cycles);
  
  spiSendValue(cycles); //Send the four characters to the display
  
  delay(1); //If we remove the slow debug statements, we need a very small delay to prevent flickering
}

//Given a number, spiSendValue chops up an integer into four values and sends them out over spi
void spiSendValue(int tempCycles)
{
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment

  SPI.transfer(tempCycles / 1000); //Send the left most digit
  tempCycles %= 1000; //Now remove the left most digit from the number we want to display
  SPI.transfer(tempCycles / 100);
  tempCycles %= 100;
  SPI.transfer(tempCycles / 10);
  tempCycles %= 10;
  SPI.transfer(tempCycles); //Send the right most digit

  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment
}
