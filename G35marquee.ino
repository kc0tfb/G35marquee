// Double-buffered G35 scrolling LED marquee - lotsa pels with little RAM
// Buffers use one byte per pixel, so we eat just over 300 bytes for 150 pels
// Addresses are stored in a different array, using one byte per pixel

// This code is tested on an AVR 32u4 (Teensy 2.0) at 16MHz/5V
// Inspired by John Riney and Jason Beeland's G35_LED project, with
// additional code (the transmit function) from the G35Arduino library

#include <Flash.h> // Font and palette are stored in Flash

// Timing delays for 16Mhz 32u4 (Teensy):
#define G35_DLONG 17    // should be ~ 20uS long
#define G35_DSHORT 7    // should be ~ 10uS long
#define G35_DEND 40     // should be ~ 30uS long

// Strand configuration
const byte G35_matrixWidth = 25;  // The width, in bulbs, of the assembled display.
const byte G35_matrixHeight = 6;  // The height, in bulbs, of the assembled display.
const byte G35_strandCount = 3;   // The number if strands used to form the display. Max = 4
const byte G35_strandLength = 50; // The number of bulbs on each strand.
const byte G35_strandPin[G35_strandCount] = {2, 3, 4}; // The digital pins assigned to the strands' data buses.

#include "G35font.h" // also includes palette
#include "G35marquee.h" // Gory guts in here

char mymsg[255]="HELLO WORLD."; // global for new things.

void setup() {
  G35_setup();
}

/* void walker(byte color=0x0F) {
  byte x=0;
  byte y=0;
  while (y < G35_matrixHeight) {
    x=0;
    while (x < G35_matrixWidth) {
      G35_buffer[x++][y]=color;
      G35_updateStrands();
      delay(20);
    };
    y++;
  };
};
*/

void serialMessage() {
    if (Serial.available()) {
    byte i=0;
    byte D;
    while (Serial.available() > 0) {      
      mymsg[i++]=Serial.read();
    }
    mymsg[i]=0;
  }
}

void loop() {
  if (Serial.available()) serialMessage();
  G35_textDisplay(mymsg); // String, Delay, Loops, Text, Background
  if (G35_runOnce == 1) { // sleep until serial data becomes available
    while (! Serial.available()) delay(10);
  }
}

