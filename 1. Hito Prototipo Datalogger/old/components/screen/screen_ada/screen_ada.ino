/**************************************************************************
Screen test for data displaying. Provides  an implementation of
showMeasurement() using the Adafruit SSD1306 library.

Based on github.com/adafruit/Adafruit_SSD1306/blob/master/examples/ssd1306_128x64_i2c/ssd1306_128x64_i2c.ino

On Arduino Nano ATMega328P:
14408 bytes progmem
624 bytes dynamic mem
---------------------------------------------------------------------------
Software License Agreement (BSD License)

Copyright (c) 2012, Adafruit Industries
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int x_text_left = 10;
const int y_title = 5;
const int y_box1 = 15;
const int y_box2 = 40;
const int text_offset = 3;

// Called by showMeasurement. Just decoration.
void _drawLayout() {
  // title
  display.setCursor(x_text_left, y_title);
  display.print(F("Last measurement:"));

  // 1st box           (x,y,dx,dy,round,color)
  display.drawRoundRect(5, y_box1, 70, 15, 5, SSD1306_WHITE);
  
  // 2nd box
  display.drawRoundRect(5, y_box2, 120, 15, 5, SSD1306_WHITE);

  //display.display(); // unnecessary delay and screen flickering
}

void showMeasurement(float value, int precission, char unit[], char tmstamp[]) {
  display.clearDisplay();
  
  _drawLayout();

  // value 1st box 
  display.setCursor(x_text_left, y_box1 + text_offset);
  display.print(value, precission);

  // outside 1st box (unit)
  display.setCursor(80, y_box1 + text_offset);
  display.print(unit);

  // value 2nd box (timestamp)
  display.setCursor(x_text_left, y_box2 + text_offset);
  display.print(tmstamp);

  //display.invertDisplay(true); // contour easier to see
  display.display();
}

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer (Adafruit splash)
  display.clearDisplay();

  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);                 // True code page 437
}

uint8_t count = 0; // rollover in ~30 seconds

void loop() {
  // special chars follow:
  // https://en.wikipedia.org/wiki/Code_page_437#Character_set
  showMeasurement(3.141592 + count, 2, "\xF8 C", "2020-02-30 10:34:56");
  count++;
  delay(100);
}
