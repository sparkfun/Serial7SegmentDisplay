/*
 11-21-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This is example code that shows how to control individual segments on each digit. Sort of like
 the Predator display at the end of the movie when he blows himself up.
 
 Note: This code expects the display to be listening at 9600bps. If your display is not at 9600bps, you can
 do a software or hardware reset. See the Wiki for more info: 
 http://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-baud
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 8 on Uno to RX on Serial7Segment
 VIN to PWR
 GND to GND
 
*/

//From https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-individual
#define SEGMENT1  0
#define SEGMENT2  1
#define SEGMENT3  2
#define SEGMENT4  3
#define SEGMENT5  4
#define SEGMENT6  5
#define SEGMENT7  6

#define DIGIT1  0x7B
#define DIGIT2  0x7C
#define DIGIT3  0x7D
#define DIGIT4  0x7E

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

void setup() {

  Serial.begin(9600);
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display

  //Here's the basic example
  Serial7Segment.write(DIGIT1);  // Control individual segments on a digit
  Serial7Segment.write( (1<<SEGMENT1) | (1<<SEGMENT4) | (1<<SEGMENT7) ); // Turn on certain segments

  Serial7Segment.write(DIGIT2);  // Control individual segments on a digit
  Serial7Segment.write( (1<<SEGMENT2) | (1<<SEGMENT5) | (1<<SEGMENT7) ); // Turn on certain segments

  Serial7Segment.write(DIGIT3);  // Control individual segments on a digit
  Serial7Segment.write( (1<<SEGMENT4) | (1<<SEGMENT5) ); // Turn on certain segments

  Serial7Segment.write(DIGIT4);  // Control individual segments on a digit
  Serial7Segment.write( (1<<SEGMENT5) | (1<<SEGMENT6) | (1<<SEGMENT7) ); // Turn on certain segments
  
  delay(1500);

  //Here's the more fun example

  //Seed random generator with analog input - nothing needs to be connected to A0  
  randomSeed(analogRead(0)); 
}

void loop() 
{
  for(int x = 0 ; x < 4 ; x++)
  {
    byte randNumber = random(0, 127); //Give me random number between 0 and 127
    Serial7Segment.write(DIGIT1 + x); //Control individual segments on a digit
    Serial7Segment.write(randNumber); //Turn on random segments
  }

  delay(1000);
}



