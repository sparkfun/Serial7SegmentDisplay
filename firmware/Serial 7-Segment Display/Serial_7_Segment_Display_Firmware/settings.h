//Pin definitions
#define SPI_CS			10
#define SPI_MOSI		11
#define SPI_MISO		12
#define SPI_SCK			13

//Define the different baud rate levels
#define BAUD_2400	0
#define BAUD_4800	1
#define BAUD_9600	2
#define BAUD_14400	3
#define BAUD_19200	4
#define BAUD_38400	5
#define BAUD_57600	6
#define BAUD_76800	7
#define BAUD_115200	8
#define BAUD_250000	9
#define BAUD_500000	10
#define BAUD_1000000	11

//Bit locations for the decimal, apostrophe and colon control
//From https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands#wiki-decimal
#define APOSTROPHE  5
#define COLON       4
#define DECIMAL4    3
#define DECIMAL3    2
#define DECIMAL2    1
#define DECIMAL1    0

#define MODE_DATA    0
#define MODE_ANALOG  1
#define MODE_COUNTER 2

const int TWI_ADDRESS_DEFAULT = 0x71;
const int BAUD_DEFAULT  = BAUD_9600;  // 9600 for 8MHz, 2x speed
const int BRIGHTNESS_DEFAULT = 100;  // 100%, full brightness
const int MODE_DEFAULT = MODE_DATA; // Watch for incoming data rather than pulses or analog voltages

//Internal EEPROM locations for the user settings
const unsigned char BRIGHTNESS_ADDRESS = 0;
const unsigned char BAUD_ADDRESS = 1;
const unsigned char TWI_ADDRESS_ADDRESS = 2;
const unsigned char MODE_ADDRESS = 3;

/* Command Modes */
const unsigned char RESET_CMD         = 0x76;
const unsigned char DECIMAL_CMD       = 0x77;
const unsigned char BLANK_CMD         = 0x78;  // 'x'
const unsigned char CURSOR_CMD        = 0x79;  // !!! NEW
const unsigned char BRIGHTNESS_CMD    = 0x7A;  // !!! NEW functionality
const unsigned char DIGIT1_CMD        = 0x7B;
const unsigned char DIGIT2_CMD        = 0x7C;
const unsigned char DIGIT3_CMD        = 0x7D;
const unsigned char DIGIT4_CMD        = 0x7E;
const unsigned char BAUD_CMD          = 0x7F;
const unsigned char TWI_ADDRESS_CMD   = 0x80;  // !!! NEW
const unsigned char FACTORY_RESET_CMD = 0x81;  // !!! NEW
const unsigned char MODE_CMD          = 0x82;  // !!! NEW

const int BUFFER_SIZE = 64;
