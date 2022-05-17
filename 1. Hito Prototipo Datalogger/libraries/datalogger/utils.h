#ifndef DATALOGGER_UTILS_H
#define DATALOGGER_UTILS_H

#include <Arduino.h>
#include <math.h> // log10, fabs

// error logging
// https://stackoverflow.com/a/5641470
#define STR(x) #x
#define MACRO_STR(x) STR(x)
#define S__LINE__ MACRO_STR(__LINE__)

#define ERROR(msg) Serial.println("[Error " S__LINE__ "]: " msg);
//#define ERROR(msg) assert((msg, false))

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
void dl_formatdate(char *buf, uint16_t y, uint8_t m, uint8_t d, uint8_t hh,
                   uint8_t mm, uint8_t ss);

// string functions
int dl_strlen(char *str);
char *dl_reverse(char *s);
char *dl_strtok(char *str, char const tok);

// number to string functions
char *dl_itoa(int num, char *s);
char *dl_u64toa(uint64_t n, char *buf);
char *dl_u8toa(uint8_t n, char *buf);
char *dl_u16toa(uint16_t n, char *buf);
char *dl_floattosci(float n, char *buf);

#endif // DATALOGGER_UTILS_H
