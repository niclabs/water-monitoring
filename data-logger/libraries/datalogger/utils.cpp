#include "utils.h"

// https://github.com/adafruit/RTClib/blob/master/RTClib.cpp
uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && y % 4 == 0)
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}

static uint64_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24UL + h) * 60 + m) * 60 + s;
}

uint64_t unixtime(uint16_t y, uint8_t m, uint8_t d, uint8_t hh, uint8_t mm,
                  uint8_t ss) {
  uint32_t t;
  uint16_t days = date2days(y - 2000, m, d);
  t = time2ulong(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000
  return t;
}

void print64(uint64_t num) {
  uint32_t less_significant = num % 10000000000;
  uint32_t most_significant = num / 10000000000;
  if (most_significant)
    Serial.print(most_significant);
  Serial.print(less_significant);
  Serial.println();
}
