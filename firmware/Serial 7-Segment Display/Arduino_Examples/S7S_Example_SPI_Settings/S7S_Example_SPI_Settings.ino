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
  SPI.setClockDivider(SPI_CLOCK_DIV16); //Slow down the master a bit

  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  SPI.transfer('v'); //Reset command
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment
}

void loop() 
{
  Serial.println("Low brightness"); //Just a debug statement
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  SPI.transfer(0x7A); // Brightness control command
  delay(1); //Small delay between bytes so S7S can buffer them
  SPI.transfer(0); // Set brightness level: 0% to 100%
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment
  delay(1); //Small delay between bytes so S7S can buffer them

  spiSendString("b000"); //Send the four characters to the display
  delay(2000); //Hang out for a bit before we go to the next brightness level

  Serial.println("Mid brightness"); //Just a debug statement
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  SPI.transfer(0x7A); // Brightness control command
  delay(1); //Small delay between bytes so S7S can buffer them
  SPI.transfer(50); // Set brightness level: 0% to 100%
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment
  delay(1); //Small delay between bytes so S7S can buffer them

  spiSendString("b050"); //Send the four characters to the display
  delay(2000); //Hang out for a bit before we go to the next brightness level

  Serial.println("High brightness"); //Just a debug statement
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  SPI.transfer(0x7A); // Brightness control command
  delay(1); //Small delay between bytes so S7S can buffer them
  SPI.transfer(100); // Set brightness level: 0% to 100%
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment
  delay(1); //Small delay between bytes so S7S can buffer them

  spiSendString("b100"); //Send the four characters to the display
  delay(2000); //Hang out for a bit before we go to the next brightness level
}

//Given a string, spiSendString chops up the string and sends out the first four characters over spi
void spiSendString(char *toSend)
{
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  SPI.transfer('v'); //Reset command
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment

  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenSegment
  for(byte x = 0 ; x < 4 ; x++)
  {
    delay(1); //Small delay between bytes so S7S can buffer them
    SPI.transfer(toSend[x]); //Send a character from the array out over I2C
  }
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenSegment
}
