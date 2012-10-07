//////////////////////////////////////
/// Serial 7-Segment Display /////////
//////////////////////////////////////
/// Arduino Firmware and Test Code ///
//////////////////////////////////////

These folders should live in your Arduino Sketchbook.

Serial 7-Segment Display: This folder contains the Serial 7-Segment Displays firmware and test/example Arduino code.

hardware/Serial7Seg: This adds a "Serial 7-Segment Display" option to the boards menu within Arduino. If you reprogram the Display, you'll need this board selected. Special pin defines were required to make use of new pins 22 and 23 (PB6:7) (in variants/standard/pins_arduino.h). These pins are available on the Serial 7-Segment Display because it runs on the ATmega328's internal 8MHz oscillator.
Digging further into this folder, you'll find the bootloader source code.

This project is Open-source!

License: Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0)
http://creativecommons.org/licenses/by-sa/3.0/