/**************************************************************************
Screen test for data displaying. Provides  an implementation of
showMeasurement() using the U8G2 library https://github.com/olikraus/u8g2

On Arduino Nano ATMega328P:
12718 bytes progmem
872 bytes dynamic mem
---------------------------------------------------------------------------
Universal 8bit Graphics Library (http://code.google.com/p/u8g2/)

Copyright (c) 2016, olikraus@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list 
  of conditions and the following disclaimer.
  
* Redistributions in binary form must reproduce the above copyright notice, this 
  list of conditions and the following disclaimer in the documentation and/or other 
  materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  s
 **************************************************************************/

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

// Nano
#define DATA_PIN A4
#define CLOCK_PIN A5

// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#ssd1306-128x64_noname-1
U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, CLOCK_PIN, DATA_PIN);


const int x_text_left = 10;
const int y_title = 5+5;
const int y_box1 = 15;
const int y_box2 = 40;
const int text_offset = 5+5;

// Called by showMeasurement. Just decoration.
void _drawLayout() {
  // title
  //u8g2.drawStr(x_text_left, y_title, "Last measurement:"); // +24 bytes progmem
  u8g2.setCursor(x_text_left, y_title);
  u8g2.print(F("Last measurement:"));

  // 1st box     (x,y,dx,dy,round)
  u8g2.drawRFrame(5, y_box1, 70, 15, 5);
  
  // 2nd box
  u8g2.drawRFrame(5, y_box2, 120, 15, 5);
}

void showMeasurement(float value, int precission, char unit[], char tmstamp[]) {
  u8g2.firstPage();
  do {
    _drawLayout();
 
    // value 1st box 
    u8g2.setCursor(x_text_left, y_box1 + text_offset);
    u8g2.print(value, precission);
  
    // outside 1st box (unit)
    u8g2.setCursor(80, y_box1 + text_offset);
    u8g2.print(unit);
  
    // value 2nd box (timestamp)
    u8g2.setCursor(x_text_left, y_box2 + text_offset);
    u8g2.print(tmstamp);

  } while ( u8g2.nextPage() );
}

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  //u8g2.enableUTF8Print(); // +142 bytes progmem
  
  // https://github.com/olikraus/u8g2/wiki/fntlistmono
  u8g2.setFont(/*u8g2_font_ncenB14_tr*/ u8g2_font_6x10_mf);
}

uint8_t count = 0;

void loop() {
  showMeasurement(3.141592 + count, 2, "o C", "2020-02-30 10:34:56");
  count++;
  
  delay(100);
}
