#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

  U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   

void setup(void) {
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
}

void loop(void) {
  u8x8.drawString(0,0,"Hello World!");
  delay(1000);
  u8x8.drawString(0,0,"See ya World!");
  delay(1000);
}
