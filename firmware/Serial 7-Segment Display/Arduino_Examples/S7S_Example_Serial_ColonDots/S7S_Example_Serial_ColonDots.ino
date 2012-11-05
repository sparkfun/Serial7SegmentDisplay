/*
 11-4-2012
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Serial7Segment is an open source seven segment display. 

 This is example code that shows how to display basic numbers and control the decimal, colon, and AM/PM dot.
 
 Note: This code works with displays the support the center colon and extra dots (AM/PM, not decimals).
 
 Note: This code expects the display to be listening at 9600bps. If your display is not at 9600bps, you can
 do a software or hardware reset. See the Wiki for more info: 
 http://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-baud
 
 To get this code to work, attached an Serial7Segment to an Arduino Uno using the following pins:
 Pin 7 on Uno (software serial RX) to TX on Serial7Segment
 Pin 8 on Uno to RX on Serial7Segment
 VIN to PWR
 GND to GND
 
*/

#include <SoftwareSerial.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

int cycles = 0;

//From https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-decimal
#define APOSTROPHE  5
#define COLON       4
#define DECIMAL4    3
#define DECIMAL3    2
#define DECIMAL2    1
#define DECIMAL1    0

long millisTimer;
byte seconds = 0;
byte minutes = 0;
byte hours = 0;
boolean amTime = true;
boolean colonOn = false;

char tempString[100]; //Used for sprintf

void setup() {

  Serial.begin(9600);
  Serial.println("OpenSegment Example Code");

  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display

  Serial7Segment.print("0822"); //Send the hour and minutes to the display

  millisTimer = millis();

  //For testing, we initialize the variables to the current time
  seconds = 57;
  minutes = 59;
  hours = 12;
}

void loop() 
{
  //Every second update the various variables and blink colon/apos/decimal
  if( (millis() - millisTimer) > 1000)
  {
    millisTimer += 1000; //Adjust the timer forward 1 second

    seconds++;
    if(seconds > 59)
    {
      seconds -= 60; //Reset seconds and increment minutes
      minutes++;
      if(minutes > 59)
      {
        minutes -= 60; //Reset minutes and increment hours
        hours++;
        if(hours > 12)
        {
          hours -= 12; //Reset hours and flip AM/PM
          if(amTime == true) 
            amTime = false; //Flip AM to PM
          else
            amTime = true;
        }
      }
    }

    //Blink the colon every other second
    if(colonOn == true)
    {
      colonOn = false;
      Serial7Segment.write(0x77);  // Decimal, colon, apostrophe control command
      Serial7Segment.write((byte) 0); // Turns off colon, apostrophoe, and all decimals
    }
    else
    {
      colonOn = true; 
      Serial7Segment.write(0x77);  // Decimal, colon, apostrophe control command
      Serial7Segment.write( (1<<APOSTROPHE) | (1<<COLON) | (1<<DECIMAL4) ); // Turns on colon, apostrophoe, and far-right decimal
    }
  }

  //Debug print the time
  sprintf(tempString, "HH:MM:SS %02d:%02d:%02d", hours, minutes, seconds);
  Serial.println(tempString);

  sprintf(tempString, "%02d%02d", minutes, seconds);
  Serial7Segment.print(tempString); //Send serial string out the soft serial port to the S7S

  delay(100);
}



