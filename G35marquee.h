// Global variables and #defines are G35_xxxx
// Functions are G35_xxxx()

#define G35_FAST 50  // lower limit, in milleseconds, between blits.
#define G35_NORM 80 // lower limit, in milleseconds, between blits.
#define G35_SLOW 110 // lower limit, in milleseconds, between blits.

volatile byte G35_fg = 0x0f; // These globals allow persistent
volatile byte G35_bg = 0x00; // on-the-fly color switching
volatile int G35_textDelay=G35_NORM; // inter-frame delay for scrolling text
volatile boolean G35_runOnce = 0; // 1 means loop forever, 0 means stop.  Must be honored with a test in your main loop();

byte G35_address[G35_matrixWidth][G35_matrixHeight]; // Addresses of individual bulbs
volatile byte G35_bulbs[G35_matrixWidth][G35_matrixHeight]; // Output buffer = The display's current value
volatile byte G35_buffer[G35_matrixWidth][G35_matrixHeight]; // Display buffer = The desired values

char G35_msg[255]="~}xaGb3c5dMeAfRgQhUiEjEk.l.m.n.o.     "; // Startup message / color test.
volatile char G35_newmsg[255]=""; // Buffer for incoming message
volatile byte G35_newmsgptr=0; // pointer for new msg length
volatile byte G35_cursor=0; // where the text gets drawn

// Macros for dW output
#define G35_ZERO(x) digitalWrite(x, LOW); delayMicroseconds(G35_DSHORT); digitalWrite(x, HIGH); delayMicroseconds(G35_DLONG);
#define G35_ONE(x) digitalWrite(x, LOW); delayMicroseconds(G35_DLONG); digitalWrite(x, HIGH); delayMicroseconds(G35_DSHORT);

#ifdef TEENSY_UART // Uart on the teensy
HardwareSerial Uart = HardwareSerial();
#endif

boolean G35_readSerial(byte d) {
  if (((d == 0x0D) | (d == 0x0A)) & (G35_newmsgptr > 0)) { // Carriage Return or Line Feed
    G35_newmsg[G35_newmsgptr++]=0x00; // null-terminate buffer
    while (G35_newmsgptr-- != 0) {G35_msg[G35_newmsgptr]=G35_newmsg[G35_newmsgptr];} // copy
    G35_newmsgptr=0;
    return(true); // flag for update
  }
  if (d < 0x20) return(false); // skip control chars
  if (d > 0x7E) return(false); // skip unprintables
  G35_newmsg[G35_newmsgptr++]=d;
  return(false);
}

boolean G35_getMessage() {
  while (Serial.available()) {
    if (G35_readSerial(Serial.read())) return(true);
  }
#ifdef TEENSY_UART // accept messages from the TTL UART as well as USB.
  while (Uart.available()) {
    if (G35_readSerial(Uart.read())) return(true);
  }
#endif
  return(false);
}

// If the character has a font table entry, return the index to it.
byte G35_fontIndex(char currChar) {
  byte ptr = (((byte)currChar) - 0x20); // ASCII conversion to table pointer
  if (ptr > G35_font.rows()) {
    return(0);
  } 
  else {
    return(ptr);
  }
}

// If the character has a font table entry, return its width.
byte G35_fontWidth(char currChar) {
  byte ptr = (((byte)currChar) - 0x20); // ASCII conversion to table pointer
  if (ptr > G35_font.rows()) {
    return(0); // unknown chars have no width
  } 
  else {
    return(G35_font[ptr][6]);
  }
}

// Shift the frame buffer one scan line to the left
void G35_shiftMatrixLeft() {
  if (G35_cursor < G35_matrixWidth) {
     G35_cursor++;
     return;
  }
  byte xpos = 0;
  byte ypos = 0;
  while(ypos < G35_matrixHeight) {
    while(xpos < (G35_matrixWidth-1)) {
      G35_buffer[xpos][ypos] = G35_buffer[(xpos + 1)][ypos];
      xpos++;
    }
    xpos = 0;
    ypos++;
  }
}

// Generate a scan line from char and position indices. Write to column at (global) G35_cursor-1.
void G35_addScanLine(char currChar, byte charPos, byte fg_, byte bg_) {
  byte charIndex = G35_fontIndex(currChar); // find pointer into font for char data
  char myChar[7] = {  // obtain character entry from font in flash memory
    G35_font[charIndex][0],    G35_font[charIndex][1],    G35_font[charIndex][2],    G35_font[charIndex][3],
    G35_font[charIndex][4],    G35_font[charIndex][5],    (G35_font[charIndex][6] -1),
  };
  charPos = (myChar[6] - charPos);
  byte colMask = ( 1<<charPos);

  G35_buffer[(G35_cursor-1)][0] = ((myChar[0] & colMask) ? fg_ : bg_ );
  G35_buffer[(G35_cursor-1)][1] = ((myChar[1] & colMask) ? fg_ : bg_ );
  G35_buffer[(G35_cursor-1)][2] = ((myChar[2] & colMask) ? fg_ : bg_ );
  G35_buffer[(G35_cursor-1)][3] = ((myChar[3] & colMask) ? fg_ : bg_ );
  G35_buffer[(G35_cursor-1)][4] = ((myChar[4] & colMask) ? fg_ : bg_ );
  G35_buffer[(G35_cursor-1)][5] = ((myChar[5] & colMask) ? fg_ : bg_ );
}

//  Transmit data to desired bulb.
void G35_sendPacket(byte address, byte col_) {
  unsigned int mycolor=G35_palette[col_]; // retrieve palette entry from flash memory
  byte pin_ = G35_strandPin[(address>>6)]; // recover strand # from 2 most significant bits of address, obtain pin #
  noInterrupts(); // the secret to avoiding timing glitches
  digitalWrite(pin_, HIGH);
  delayMicroseconds(G35_DSHORT);

  // LED Address
  if (address & 0x20) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (address & 0x10) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (address & 0x08) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (address & 0x04) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (address & 0x02) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (address & 0x01) { G35_ONE(pin_); } else { G35_ZERO(pin_); }

  // Intensity encoded in 4 highest bits of color
  if (mycolor & 0x8000) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x4000) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x2000) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x1000) { G35_ONE(pin_); } else { G35_ZERO(pin_); }

  // fudge four least significant bits of intensity
  G35_ZERO(pin_); 
  G35_ZERO(pin_); 
  G35_ZERO(pin_); 
  G35_ZERO(pin_);

  // Blue encoded in 4 lowest bits of color
  if (mycolor & 0x0008) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0004) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0002) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0001) { G35_ONE(pin_); } else { G35_ZERO(pin_); }

  // Green encoded in next lowest 4 bits
  if (mycolor & 0x0080) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0040) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0020) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0010) { G35_ONE(pin_); } else { G35_ZERO(pin_); }

  // Red encoded in next lowest 4 bits
  if (mycolor & 0x0800) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0400) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0200) { G35_ONE(pin_); } else { G35_ZERO(pin_); }
  if (mycolor & 0x0100) { G35_ONE(pin_); } else { G35_ZERO(pin_); }

  digitalWrite(pin_, LOW);
  delayMicroseconds(G35_DEND);
  interrupts(); // don't forget to turn interrupts back on
}

// Fill the frame buffer with the specified color.
void G35_fillDisplay(byte color_) {
  byte xpos = 0;  byte ypos = 0;
  while(ypos < G35_matrixHeight) {
    while(xpos < G35_matrixWidth) {
      G35_buffer[xpos][ypos] = color_;
      xpos++;
    }
    xpos = 0;
    ypos++;
  }
  G35_cursor=G35_matrixWidth;
}

// Compare frame buffer and output buffer. If they differ, update output buffer and transmit an update.
void G35_updateStrands() {
  byte xpos = 0;
  byte ypos = 0;
  while(ypos < G35_matrixHeight) {
    while(xpos < G35_matrixWidth) {
      if(G35_bulbs[xpos][ypos] != G35_buffer[xpos][ypos]) {
        G35_bulbs[xpos][ypos] = G35_buffer[xpos][ypos];
        G35_sendPacket(G35_address[xpos][ypos], G35_bulbs[xpos][ypos]);
      }
      xpos++;
    }
    xpos = 0;
    ypos++;
  }
}

byte G35_newfg(byte c) {
  if (c == G35_bg) return(G35_bg);
  c++;
  if (c > 0x0F) c=0x00;
  if (c == G35_bg) c++;
  return(c);
}

void G35_colorCycle() { // revolve foreground colors in display buffer
  byte i=0;
  while (i < 0x0F) {
    byte xpos = 0;  byte ypos = 0;
    while(ypos < G35_matrixHeight) {
      while(xpos < G35_matrixWidth) {
        byte c=G35_newfg(G35_buffer[xpos][ypos]);
        G35_buffer[xpos][ypos] = c;
        xpos++;
      }
      xpos = 0;
      ypos++;
    }
    delay(G35_textDelay*2);
    G35_updateStrands();
    i++;
  }
}
      

//Scrolls text across the screen.
//  displayText is the string you wish to be displayed.
//  refreshDelay is minimum msec between refreshes.
//  loops is the number of times to scroll the message before returning to the calling function.
//  fg and bg are palette entries for foreground (text) and background color.

void G35_textDisplay(String displayText, byte loops = 1, byte fg_ = G35_fg, byte bg_ = G35_bg) {
  byte charPos = 0;
  byte loopCount = 0;
  unsigned int textPos = 0;
  unsigned int textLength = displayText.length();
  unsigned long lastDispTime = 0;
  unsigned long currTime = 0;
  while((loops == 0) || (loopCount < loops)) {
    textPos = 0; // index into the string displayed
    charPos = 0; // desired column of character (leftmost = 0)
    while (textPos < textLength) {
#ifdef LED_PIN
      digitalWrite(LED_PIN, (! digitalRead(LED_PIN))); // heartbeat
#endif
      if (G35_getMessage()) return; // check for input
      if ((displayText[textPos] & 0xF0) == 0x60) { // new FG color for "`a-o"
        fg_ = (displayText[textPos] & 0x0F); // 0x00-0x0
        G35_fg = fg_;
        textPos++;
        continue;
      }
      if ((displayText[textPos] & 0xF0) == 0x70) {
        switch (displayText[textPos]) {
          case 0x70: G35_bg=fg_; G35_fg=bg_; fg_=G35_fg; bg_=G35_bg; break; // p - swap fore/background colors
          case 0x71: G35_fillDisplay(bg_); break; // q   Fill display buffer with background color
          case 0x72: G35_fillDisplay(fg_); break; // r   Fill display buffer with foreground color
          case 0x73: G35_updateStrands();  break; // s   Force an update (good idea after q or r)
          case 0x74: delay(1000);          break; // t   Pause 1 second
          case 0x75: delay(4000);          break; // u   Pause 4 second
          case 0x76: G35_colorCycle();     break; // v   Twinkle/cycle foreground color
          case 0x77: G35_textDelay=0;        break; // w update as fast as possible   
          case 0x78: G35_textDelay=G35_FAST; break; // x Speed: fast
          case 0x79: G35_textDelay=G35_NORM; break; // y Speed: normal
          case 0x7A: G35_textDelay=G35_SLOW; break; // z Speed: slow
          case 0x7B: G35_runOnce=false;      break; // { Run forever
          case 0x7C: G35_cursor=0;           break; // | Move cursor to first column
          case 0x7D: G35_runOnce=true;       break; // } Show text only once. *(see main loop())
          case 0x7E: G35_textDelay=G35_NORM;        // ~ Restore defaults...
                     G35_runOnce=false;             // Loop forever 
                     fg_=0x0F; G35_fg=fg_;          // White foreground
                     bg_=0x00; G35_bg=bg_;          // Black background
                     break;
        }
        textPos++;
        continue;
      }
      currTime = millis();
      if ((lastDispTime == 0) || (currTime >= (lastDispTime + G35_textDelay))) {
        G35_shiftMatrixLeft();
        // Serial.print("Drawing ");
        // Serial.print(displayText[textPos]);
        // Serial.print("/");
        // Serial.print(charPos, DEC);
        // Serial.print(" at column ");
        // Serial.println(G35_cursor, DEC);
        G35_addScanLine(displayText[textPos], charPos++, fg_, bg_);
        if(charPos >= G35_fontWidth(displayText[textPos])) {
          charPos = 0;
          textPos++;
        }
        G35_updateStrands();
        lastDispTime = currTime;
      }
    }
    loopCount++;
  }
}

// Populates buffers with address/strand data and updates bulbs with position data
// Must be called by setup() prior to other G35_ functions.
void G35_setup() {
  byte addr = 0;
  byte xpos = 0;
  byte ypos = 0;
  byte currentStrand = 0;
  byte strandMask = 0;
  while (addr < G35_strandCount) {
    digitalWrite(G35_strandPin[addr], LOW);
    pinMode(G35_strandPin[addr], OUTPUT);
    addr++;
  }
  while (currentStrand < G35_strandCount) {
    strandMask = (currentStrand<<6);
    addr = 0;
    while (addr < G35_strandLength) {
      G35_address[xpos][ypos] = (addr | strandMask);
      G35_bulbs[xpos][ypos] = 0;
      G35_sendPacket(G35_address[xpos][ypos], G35_bulbs[xpos][ypos]);
      delay(1);
      if ((ypos & 1) == 0) {
        if (xpos < (G35_matrixWidth - 1)) { xpos++; } else { ypos++; }
      } else { if (xpos > 0) { xpos--; } else { ypos++; } }
      addr++;
    }
    currentStrand++;
  }
}

