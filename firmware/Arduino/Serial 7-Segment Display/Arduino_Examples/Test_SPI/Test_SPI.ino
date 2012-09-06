#include <SPI.h>

const int slaveSelectPin = 9;
int digits[4] = {0, 0, 0, 0};

void setup() 
{
  pinMode (slaveSelectPin, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV64);  // Default div is 4, but that's too fast
  Serial.begin(9600);
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer('v');
  digitalWrite(slaveSelectPin, HIGH);
  delay(2000);
}

void loop()
{
  if (Serial.available())
  {
    digitalWrite(slaveSelectPin, LOW);
    SPI.transfer(Serial.read());
    digitalWrite(slaveSelectPin, HIGH);
  }
  /*
  digits[0]++;
  for (int i=0; i<4; i++)
  {
    if (digits[i] > 9)
    {
      digits[i] = 0;
      digits[(i+1)%4]++;
    }
  }
  Serial.print(digits[3]);
  Serial.print(digits[2]);
  Serial.print(digits[1]);
  Serial.println(digits[0]);
  
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(digits[3]);
  SPI.transfer(digits[2]);
  SPI.transfer(digits[1]);
  SPI.transfer(digits[0]);
  digitalWrite(slaveSelectPin, HIGH);
  delay(100);
  /*int temp = millis()/100;
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer('v');
  SPI.transfer(temp/1000);
  SPI.transfer((temp-temp/1000)/100);
  SPI.transfer((temp-temp/1000-temp/100)/10);
  SPI.transfer((int)(temp-temp/1000-temp/100-temp/10));
  SPI.transfer('w');
  SPI.transfer(0x2);
  digitalWrite(slaveSelectPin, HIGH);
  delay(500);*/
}
