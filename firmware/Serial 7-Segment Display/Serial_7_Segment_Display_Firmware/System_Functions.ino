/* 
 Serial 7 Segment Display : System Functions

 See main file for license and information.

 These are all the lower level system functions that allow Serial 7 Segment to run
 
 This includes the ISRs and setup functions.
 
 */

// SPI byte received interrupt routine
ISR(SPI_STC_vect)
{
  noInterrupts();  // don't be rude! I'll be quick...

  unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
  unsigned char c = SPDR;  // Read data byte into c, from SPI data register

  if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
  {
    buffer.data[buffer.head] = c;  // Store the data into the buffer's head
    buffer.head = i;  // update buffer head, since we stored new data
  }

  interrupts();  // Fine, you were saying?
}

// The display data is updated on a Timer interrupt
ISR(TIMER1_COMPA_vect)
{
  noInterrupts();

  // if head and tail are not equal, there's data to be read from the buffer
  if (buffer.head != buffer.tail)
    updateBufferData();  // updateBufferData() will update the display info, or peform special commands

  interrupts();
}

// This is effectively the UART0 byte received interrupt routine
// But not quite: serialEvent is only called after each loop() interation
void serialEvent()
{
  while (Serial.available()) 
  {
    unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
    unsigned char c = Serial.read();  // Read data byte into c, from UART0 data register

    if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
    {
      buffer.data[buffer.head] = c;  // Store the data into the buffer's head
      buffer.head = i;  // update buffer head, since we stored new data
    }
  }
}

// I2C byte receive interrupt routine
// Note: this isn't an ISR. I'm using wire library (because it just works), so
// Wire.onReceive(twiReceive); should be called
void twiReceive(int rxCount)
{
  while(Wire.available())  // Do this while data is available in Wire buffer
  {
    unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
    unsigned char c = Wire.read();  // Read data byte into c, from Wire data buffer

    if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
    {
      buffer.data[buffer.head] = c;  // Store the data into the buffer's head
      buffer.head = i;  // update buffer head, since we stored new data
    }    
  }
}

// seutpTimer(): Set up timer 1, which controls interval reading from the buffer
void setupTimer()
{
  // Timer 1 is se to CTC mode, 16-bit timer counts up to 0xFF
  TCCR1B = (1<<WGM12) | (1<<CS10);
  OCR1A = 0x00FF;
  TIMSK1 = (1<<OCIE1A);  // Enable interrupt on compare
}

//Sets up the hardware pins to control the 7 segments and display type
void setupDisplay()
{
  //Determine the display brightness
  byte settingBrightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  if(settingBrightness > BRIGHTNESS_DEFAULT) {
    settingBrightness = BRIGHTNESS_DEFAULT; //By default, unit will be brightest
    EEPROM.write(BRIGHTNESS_ADDRESS, settingBrightness);
  }
  myDisplay.SetBrightness(settingBrightness); //Set the display to 100% bright

  // Set the initial state of displays and decimals 'x' =  off
  display.digits[0] = 'x';
  display.digits[1] = 'x';
  display.digits[2] = 'x';
  display.digits[3] = 'x';
  display.decimals = 0x00;  // Turn all decimals off
  display.cursor = 0;  // Set cursor to first (left-most) digit

  buffer.head = 0;  // Initialize buffer values
  buffer.tail = 0;  

  //Declare what pins are connected to the digits

  //This pinout is for OpenSegment PCB layout
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#if DISPLAY_TYPE == OPENSEGMENT
  int digit1 = 9; //Pin 12 on my 4 digit display
  int digit2 = 16; //Pin 9 on my 4 digit display
  int digit3 = 17; //Pin 8 on my 4 digit display
  int digit4 = 3; //Pin 6 on my 4 digit display

  //Declare what pins are connected to the segments
  int segA = 14; //Pin 11 on my 4 digit display
  int segB = 2; //Pin 7 on my 4 digit display
  int segC = 8; //Pin 4 on my 4 digit display
  int segD = 6; //Pin 2 on my 4 digit display
  int segE = 7; //Pin 1 on my 4 digit display
  int segF = 15; //Pin 10 on my 4 digit display
  int segG = 4; //Pin 5 on my 4 digit display
  int segDP= 5; //Pin 3 on my 4 digit display

  int numberOfDigits = 4; //Do you have a 2 or 4 digit display?

  //The 1" SparkFun displays are common cathode but because of the PNP and NPN
  //transistor configuration we are using, we need inverted signals:
  //1 to turn a digit on, 0 to turn a segment on
  //This is the same as a common anode setup.
  int displayType = COMMON_ANODE; 

  //Initialize the SevSeg library with all the pins needed for this type of display
  myDisplay.Begin(displayType, numberOfDigits, 
  digit1, digit2, digit3, digit4, 
  segA, segB, segC, segD, segE, segF, segG, 
  segDP);

#endif
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //This pinout is for the original Serial7Segment layout
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#if DISPLAY_TYPE == S7S
  int digit1 = 16; // DIG1 = A2/16 (PC2)
  int digit2 = 17; // DIG2 = A3/17 (PC3)
  int digit3 = 3;  // DIG3 = D3 (PD3)
  int digit4 = 4;  // DIG4 = D4 (PD4)

  //Declare what pins are connected to the segments
  int segA = 8;  // A = D8 (PB0)
  int segB = 14; // B = A0 (PC0)
  int segC = 6;  // C = D6 (PD6), shares a pin with colon cathode
  int segD = A1; // D = A1 (PC1)
  int segE = 23; // E = PB7 (not a standard Arduino pin: Must add PB7 as digital pin 23 to pins_arduino.h)
  int segF = 7;  // F = D7 (PD6), shares a pin with apostrophe cathode
  int segG = 5;  // G = D5 (PD5)
  int segDP= 22; //DP = PB6 (not a standard Arduino pin: Must add PB6 as digital pin 22 to pins_arduino.h)

  int digitColon = 2; // COL-A = D2 (PD2) (anode of colon)
  int segmentColon = 6; // COL-C = D6 (PD6) (cathode of colon), shares a pin with C
  int digitApostrophe = 9; // APOS-A = D9 (PB1) (anode of apostrophe)
  int segmentApostrophe = 7; // APOS-C = D7 (PD7) (cathode of apostrophe), shares a pin with F

  int numberOfDigits = 4; //Do you have a 2 or 4 digit display?

  int displayType = COMMON_ANODE; //SparkFun 10mm height displays are common anode

  //Initialize the SevSeg library with all the pins needed for this type of display
  myDisplay.Begin(displayType, numberOfDigits, 
  digit1, digit2, digit3, digit4, 
  digitColon, digitApostrophe, 
  segA, segB, segC, segD, segE, segF, segG, 
  segDP,
  segmentColon, segmentApostrophe);
#endif
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  
  //This pinout is for the original Serial 7 Segment Shield
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#if DISPLAY_TYPE == S7SHIELD
  int digit1 = 16; // DIG1 = A2/16 (PC2)
  int digit2 = 17; // DIG2 = A3/17 (PC3)
  int digit3 = 3;  // DIG3 = D3 (PD3)
  int digit4 = 4;  // DIG4 = D4 (PD4)

  //Declare what pins are connected to the segments
  int segA = 8;  // A = D8 (PB0)
  int segB = 14; // B = A0 (PC0)
  int segC = 6;  // C = D6 (PD6), shares a pin with colon cathode
  int segD = A1; // D = A1 (PC1)
  int segE = 23; // E = PB7 (not a standard Arduino pin: Must add PB7 as digital pin 23 to pins_arduino.h)
  int segF = 7;  // F = D7 (PD6), shares a pin with apostrophe cathode
  int segG = 5;  // G = D5 (PD5)
  int segDP= 22; //DP = PB6 (not a standard Arduino pin: Must add PB6 as digital pin 22 to pins_arduino.h)

  int numberOfDigits = 4; //Do you have a 2 or 4 digit display?

  int displayType = COMMON_CATHODE; 

  //Initialize the SevSeg library with all the pins needed for this type of display
  myDisplay.Begin(displayType, numberOfDigits, 
  digit1, digit2, digit3, digit4, 
  segA, segB, segC, segD, segE, segF, segG, 
  segDP);
#endif
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
}

//This sets up the UART with the stored baud rate in EEPROM
void setupUART()
{
  //Read what the current UART speed is from EEPROM memory
  //Default is 9600
  byte settingUARTSpeed = EEPROM.read(BAUD_ADDRESS);
  if(settingUARTSpeed > BAUD_1000000) //Check to see if the baud rate has ever been set
  {
    settingUARTSpeed = BAUD_9600; //Reset UART to 9600 if there is no baud rate stored
    EEPROM.write(BAUD_ADDRESS, settingUARTSpeed);
  }

  //Initialize the UART
  switch(settingUARTSpeed)
  {
    case(BAUD_2400):
    Serial.begin(2400);
    break;
    case(BAUD_4800):
    Serial.begin(4800);
    break;
    case(BAUD_9600):
    Serial.begin(9600);
    break;
    case(BAUD_14400):
    Serial.begin(14400);
    break;
    case(BAUD_19200):
    Serial.begin(19200);
    break;
    case(BAUD_38400):
    Serial.begin(38400);
    break;
    case(BAUD_57600):
    Serial.begin(57600);
    break;
    case(BAUD_76800):
    Serial.begin(76800);
    break;
    case(BAUD_115200):
    Serial.begin(115200);
    break;
    case(BAUD_250000):
    Serial.begin(250000);
    break;
    case(BAUD_500000):
    Serial.begin(500000);
    break;
    case(BAUD_1000000):
    Serial.begin(1000000);
    break;
  default:
    //We should never reach this state, but if we do
    Serial.begin(9600);    
    break;
  }

}

//This function reads the MODE setting from EEPROM and checks to see if there are 
//any hardware settings (closed jumpers for example) that puts the device into a 
//certain mode. Available modes are regular, analog meter, and counter modes.
void setupMode()
{
  deviceMode = EEPROM.read(MODE_ADDRESS); // Read the mode the device should be in
  
  if (deviceMode > MODE_COUNTER)
  { // If the mode is invalid, goto default mode
    deviceMode = MODE_DEFAULT;
    EEPROM.write(MODE_ADDRESS, MODE_DEFAULT);
  }
  
#if DISPLAY_TYPE == OPENSEGMENT
  //See if any solder jumpers have been closed

  //Arduino doesn't really support PB6 and PB7 and GPIOs (normally the 16MHz
  //crystal is there) so let's do it the old school way
  //digitalWrite(JUMPER_COUNTER, HIGH); //Enable internal pullup
  //pinMode(JUMPER_METER, INPUT_PULLUP);
  
  DDRB &= 0b00111111; //Set PB7 and PB6 to inputs
  PORTB |= 0b11000000; //Enable external pull ups
  
  Serial.println("Jumper check:");
  if( (PINB & 1<<6) == 0) //If counter pin is low
  {
    deviceMode = MODE_COUNTER;
    Serial.println("PB6 jumper closed");
  }
  else if( (PINB & 1<<7) == 0) //digitalRead(JUMPER_METER) == LOW)
  {
    deviceMode = MODE_ANALOG;
    Serial.println("PB7 jumper closed");
  }
#endif

}

//This sets up the two analog inputs
void setupAnalog()
{
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
}

// setupSPI(): Initialize SPI, sets up hardware pins and enables spi and receive interrupt
// SPI is set to MODE 0 (CPOL=0, CPHA=0), slave mode, LSB first
void setupSPI()
{
  pinMode(SPI_SCK, INPUT);
  pinMode(SPI_MOSI, INPUT);
  pinMode(SPI_CS, INPUT_PULLUP);

  SPCR = (1<<SPIE) | (1<<SPE);  // Enable SPI interrupt, enable SPI
  // DORD = 0, LSB First
  // MSTR = 0, SLAVE
  // CPOL = 0, sck low when idle                  } MODE 0
  // CPHA = 0, data sampled on leading clock edge } MODE 0
  // SPR1:0 = 0, no effect (slave mode)
}

// setupTWI(): initializes I2C (err TWI! TWI! TWI!, can't bang that into my head enough)
// I'm using the rock-solid Wire library for this. We'll initialize TWI, setup the address,
// and tell it what interrupt function to jump to when data is received.
void setupTWI()
{
  unsigned char twiAddress;

  twiAddress = EEPROM.read(TWI_ADDRESS_ADDRESS);  // read the TWI address from 

  if ((twiAddress == 0) || (twiAddress > 0x7F))  
  { // If the TWI address is invalid, use a default address
    twiAddress = TWI_ADDRESS_DEFAULT;
    EEPROM.write(TWI_ADDRESS_ADDRESS, TWI_ADDRESS_DEFAULT);
  }

  Wire.begin(twiAddress);  // Initialize Wire library as slave at twiAddress address
  Wire.onReceive(twiReceive);  // setup interrupt routine for when data is received
}

//Check to see if we need an emergency system reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then reset the system settings
void checkEmergencyReset(void)
{
  pinMode(0, INPUT); //Turn the RX pin into an input
  digitalWrite(0, HIGH); //Push a 1 onto RX pin to enable internal pull-up

  //Quick pin check
  if(digitalRead(0) == HIGH) return;

  myDisplay.SetBrightness(100); //Set display to 100% brightness during emergency reset so we can see it

  //Wait 2 seconds, displaying reset-ish things while we wait
  for(uint8_t i = 0 ; i < 10 ; i++)
  {
    constantDisplay("____", 200);
    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore

    constantDisplay("----", 200);
    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore
  }		

  //If we make it here, then RX pin stayed low the whole time
  setDefaultSettings(); //Reset baud rate, brightness setting and TWI address

    //Now sit in a loop indicating system is now at 9600bps
  while(digitalRead(0) == LOW)
  {
    constantDisplay("000-", 500);
    constantDisplay("00-0", 500);
    constantDisplay("0-00", 500);
    constantDisplay("-000", 500);
  }

  //Once we breakout of this loop (pin on RX is removed), system will init with new default settings
}

//Given a string, displays it costantly for a given amount of time
void constantDisplay (char *theString, long amountOfTime)
{
  long startTime = millis();
  while( (millis() - startTime) < amountOfTime)
    myDisplay.DisplayString(theString, 0); //(numberToDisplay, decimal point location)
}

// In case of emergency, resets all the system settings to safe values
// This will reset baud, TWI address, brightness, and mode to default values
void setDefaultSettings(void)
{
  //Reset UART to 9600bps
  EEPROM.write(BAUD_ADDRESS, BAUD_DEFAULT);

  //Reset system brightness to the brightest level
  EEPROM.write(BRIGHTNESS_ADDRESS, BRIGHTNESS_DEFAULT);
  myDisplay.SetBrightness(BRIGHTNESS_DEFAULT);

  //Reset the I2C address to the default 0x71
  EEPROM.write(TWI_ADDRESS_ADDRESS, TWI_ADDRESS_DEFAULT);

  //Reset the mode to the default data interface
  EEPROM.write(MODE_ADDRESS, MODE_DEFAULT);
  deviceMode = MODE_DEFAULT;
}
