const int TWI_ADDRESS_DEFAULT = 0x71;
const int BAUD_DEFAULT  = 103;  // 9600 for 8MHz, 2x speed
const int BRIGHTNESS_DEFAULT = 255;  // Full brightness

/* EEPROM ADDRESSES */
const unsigned char BRIGHTNESS_ADDRESS = 0;
const unsigned char BAUD_ADDRESS_L = 2;
const unsigned char BAUD_ADDRESS_H = 3;
const unsigned char TWI_ADDRESS_ADDRESS = 4;  // !!! NEW

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

const int BUFFER_SIZE = 64;
