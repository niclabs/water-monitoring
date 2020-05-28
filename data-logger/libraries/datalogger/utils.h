#ifndef DATALOGGER_UTILS_H
#define DATALOGGER_UTILS_H

#include <Arduino.h>

// https://github.com/adafruit/RTClib/blob/master/RTClib.cpp
#define SECONDS_FROM_1970_TO_2000                                              \
  946684800 ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

// "private"
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d);
static uint64_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s);

// "public"
uint8_t bcd2bin(uint8_t val);
const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30,
                                       31, 31, 30, 31, 30};
uint64_t unixtime(uint16_t y, uint8_t m, uint8_t d, uint8_t hh, uint8_t mm,
                  uint8_t ss);
void print64(uint64_t num);

#endif // DATALOGGER_UTILS_H
