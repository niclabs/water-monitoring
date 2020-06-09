/**************************************************************************
Screen test for data displaying, using the RAM-less* U8x8 library
https://github.com/olikraus/u8g2

* "U8x8lib is the direct write, zero RAM, text only library part of U8g2lib"

On Arduino Nano ATMega328P:
6728 bytes progmem
366 bytes dynamic mem
**************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <U8x8lib.h>

// Nano
#define DATA_PIN A4
#define CLOCK_PIN A5

// https://github.com/olikraus/u8g2/wiki/u8x8setupcpp#ssd1306-128x64_noname-1
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(CLOCK_PIN, DATA_PIN);

/* u8x8.begin() is required and will sent the setup/init sequence to the display */
void setup(void) {
  u8x8.begin();
}

uint8_t t = 0;

void loop(void) {
  // Usage: https://github.com/olikraus/u8g2/wiki/u8x8reference
  // Screen fits only 16x8 chars 
  
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,0, "Last measurement");
  u8x8.setCursor(1,1);
  u8x8.print(t++);
  u8x8.drawString(0,3, "at");
  u8x8.drawString(1,4, "2020-02-30");
  u8x8.drawString(1,5, "12:34:56");
  delay(1000);
}
