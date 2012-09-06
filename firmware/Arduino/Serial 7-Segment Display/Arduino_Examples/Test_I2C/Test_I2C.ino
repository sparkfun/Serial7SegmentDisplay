#include <Wire.h>

const int DISPLAY_COUNT = 1;

//int address[DISPLAY_COUNT] = {0x71, 0x72, 0x73, 0x74};
int address[DISPLAY_COUNT] = {0x71};

int digs[4*DISPLAY_COUNT] = {0, 0, 0, 0};

void setup()
{
  randomSeed(analogRead(A0));
  for (int i=0; i<(4*DISPLAY_COUNT); i++)
  {
    digs[i] = random(9);
  }
  //Serial.begin(9600);
  Wire.begin();
  for (int i=0; i<4; i++)
  {
    Wire.beginTransmission(address[i]);
    Wire.write('v');
    Wire.endTransmission();
  }
}

void loop()
{
  delay(1);
  digs[0]+=1;
  digs[1]+=1;
  digs[2]+=1;
  digs[3]+=1;
  digs[4]+=1;
  digs[5]+=9;
  for (int n=0; n<16; n++)
  {
    if (digs[n] >=10)
    {
      digs[n] = 0;
      digs[(n+1)%16]++;
    }
  }
  Wire.beginTransmission(address[3]);
  Wire.write(digs[3]);
  Wire.write(digs[2]);
  Wire.write(digs[1]);
  Wire.write(digs[0]);
  Wire.endTransmission();
  Wire.beginTransmission(address[2]);
  Wire.write(digs[7]);
  Wire.write(digs[6]);
  Wire.write(digs[5]);
  Wire.write(digs[4]);
  Wire.endTransmission();
  Wire.beginTransmission(address[1]);
  Wire.write(digs[11]);
  Wire.write(digs[10]);
  Wire.write(digs[9]);
  Wire.write(digs[8]);
  Wire.endTransmission();
  Wire.beginTransmission(address[0]);
  Wire.write(digs[15]);
  Wire.write(digs[14]);
  Wire.write(digs[13]);
  Wire.write(digs[12]);
  Wire.endTransmission();
  /*if (Serial.available())
  {
    Wire.beginTransmission(address);
    Wire.write(Serial.read());
    Wire.endTransmission();
  }*/
}
