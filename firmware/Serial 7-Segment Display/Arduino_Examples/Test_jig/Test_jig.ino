#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>

// Default I2C address of 7Segment Display
const byte DISPLAY_ADDRESS = 0x71;

// Test jig pin defines
const int slaveSelect = 7;
const int statLed = A0;
const int softwareTx = 4;

// Initialize SoftwareSerial
SoftwareSerial seg7(5, 4);

void setup()
{
  randomSeed(analogRead(A1));
  seg7.begin(9600);  // Start software serial
}

void loop()
{
  if (digitalRead(slaveSelect) == 0)
  {
    delay(100);
    i2cTest();
    serialTest();
  }
}

void spiTest()
{
  pinMode(slaveSelect, INPUT);
}

void serialTest()
{
  //seg7.write('v');  // Clear display
  //seg7.write(0x77);  // Decmial command
  //seg7.write(0xFF);  // All decimals on
  //seg7.write(random(9));  // Digit 1
  //seg7.write(random(9));  // Digit 2
  seg7.write(8);  // Digit 3
  seg7.write(8);  // Digit 4  
}

void i2cTest()
{
  Wire.begin();
  Wire.beginTransmission(DISPLAY_ADDRESS);
  Wire.write('v');  // Clear display
  Wire.write(0x77);  // Decmial command
  Wire.write(0xFF);  // All decimals on
  Wire.write(8);  // Digit 1
  Wire.write(8);  // Digit 2
  //Wire.write(random(9));  // Digit 3
  //Wire.write(random(9));  // Digit 4
  Wire.endTransmission();
}
