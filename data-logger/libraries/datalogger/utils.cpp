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

// Sets the `buf` as a "YYYY-MM-DD hh:mm:ss" timestamp.
// Needs a buffer of length 20.
void dl_formatdate(char *buf, uint16_t y, uint8_t m, uint8_t d, uint8_t hh,
                   uint8_t mm, uint8_t ss) {
  char *current_pos = buf;
  // year
  dl_u16toa(y, current_pos);
  current_pos += 4; // assume always YYYY
  *current_pos++ = '-';
  // month
  *current_pos++ = m < 10 ? '0' : '1';
  *current_pos++ = m + '0';
  *current_pos++ = '-';
  // day
  if (d < 10)
    *current_pos++ = '0';
  dl_u8toa(d, current_pos);
  current_pos += d < 10 ? 1 : 2;

  *current_pos++ = ' ';
  // hour
  if (hh < 10)
    *current_pos++ = '0';
  dl_u8toa(hh, current_pos);
  current_pos += hh < 10 ? 1 : 2;
  *current_pos++ = ':';

  if (mm < 10)
    *current_pos++ = '0';
  dl_u8toa(mm, current_pos);
  current_pos += mm < 10 ? 1 : 2;
  *current_pos++ = ':';

  if (ss < 10)
    *current_pos++ = '0';
  dl_u8toa(ss, current_pos);
  current_pos += ss < 10 ? 1 : 2;
  *current_pos = '\0';
}

void print64(uint64_t num) {
  uint32_t less_significant = num % 10000000000;
  uint32_t most_significant = num / 10000000000;
  if (most_significant)
    Serial.print(most_significant);
  Serial.print(less_significant);
  Serial.println();
}

// **************** STRING FUNCTIONS ****************

int dl_strlen(char *str) {
  int len = 0;
  while (*str) {
    len++;
    str++;
  }
  return len;
}

// Reverse a string in place
// From K&R's
char *dl_reverse(char *s) {
  int c, i, j;
  for (i = 0, j = dl_strlen(s) - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
  return s;
}

// dl_strtok(str, tok) explodes the string `str` using `tok` as a separator
// token. If str is NULL, the last string upon which was called will continue
// to be exploded.
// Lightweight version of strtok (`tok` is a char instead of char*)
char *dl_strtok(char *str, const char tok) {
  static char *old_str;
  if (str == NULL)
    str = old_str;

  char *s = str;
  while (*s) {
    if (*s == tok) {
      *s = '\0';
      old_str = s + 1;
      return str;
    }
    s++;
  }

  if (!*str) // no `tok`s left
    return NULL;
  return old_str;
}

// **************** NUMBER TO STRING FUNCTIONS ****************

// itoa from K&R's
// using local implementation instead of itoa saves 38 bytes in SRAM.
// INT_MIN correction from https://stackoverflow.com/a/39967439
char *dl_itoa(int num, char *s) {
  int i, sign;
  unsigned int n;

  if ((sign = num) < 0) {
    n = -num;
  } else {
    n = num;
  }

  i = 0;
  do {                     /* generate digits in reverse order */
    s[i++] = n % 10 + '0'; /* get next digit */
  } while ((n /= 10) > 0); /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  return dl_reverse(s);
}

char *dl_u64toa(uint64_t n, char *s) {
  int i = 0;
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);
  s[i] = '\0';
  return dl_reverse(s);
}

char *dl_u8toa(uint8_t n, char *s) {
  int i = 0;
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);
  s[i] = '\0';
  return dl_reverse(s);
}

char *dl_u16toa(uint16_t n, char *s) {
  int i = 0;
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);
  s[i] = '\0';
  return dl_reverse(s);
}

/*
 * IEEE754 binary32:
 * - 1 bit sign
 * - 8 bits exponent_part
 * - 23 bits mantissa
 *
 * https://en.wikipedia.org/wiki/Single-precision_floating-point_format
 * binary32: 6 to 9 decimal digits precision.
 *
 *                    1         2         3
 *           1        0         0         0
 *         0b10000000000000000000000000000000
 *           ^ [exp] ^      [mantissa]
 *          sgn
 * **/
#define DL_FLOAT_SIGN_MASK (1 << 31)

// Float to Scientific notation-number string
// At most, will use 14 chars in `buf`
// (9 digits, 1 point, 1 'e', 2 minus signs, 1 \0)
// Ex: "-8.52421e-123"
// Sadly, some numbers will not comply with scientific notation:
// Ex -0.00324 -> "-0.32399e-2" has a leading zero.
char *dl_floattosci(float n, char *buf) {
  // as casting a float to any integral number type will round it instead
  // of considering is bytes, we indirectly cast it using *(uint32_t*)&n
  uint8_t exponent_part = (((*(uint32_t *)&n)) >> 23) & 0xFF;

  // Catch special cases
  if (!exponent_part) { // `n` is number 0
    buf[0] = '0';
    buf[1] = '\0';
    return buf;
  }

  if (exponent_part == 0xFF) { // `n` is NaN (+/-Inf or div error)
    buf[0] = buf[2] = 'N';
    buf[1] = 'a';
    buf[3] = '\0';
    return buf;
  }

  uint8_t sign = ((*(uint32_t *)&n) & DL_FLOAT_SIGN_MASK) >> 31;

  if (sign) {
    *buf++ = '-';
    *(uint32_t *)&n &= ~DL_FLOAT_SIGN_MASK;
  }

  int exponent = (int)log10(fabs(n));     // in base 10, not IEEE754's base 2
  float mantissa = n / pow(10, exponent); // 1 <= mantissa < 10

  char decimal_digit_mantissa = (char)mantissa;
  *buf++ = decimal_digit_mantissa + '0';

  // get next mantissa digit
  mantissa *= 10;
  while (mantissa > 10)
    mantissa -= 10;

  *buf++ = '.'; // decimal point

  for (uint8_t i = 0; i < 5; i++) {
    decimal_digit_mantissa = (char)mantissa;
    *buf++ = decimal_digit_mantissa + '0';

    mantissa *= 10;
    while (mantissa > 10)
      mantissa -= 10;
  }

  *buf++ = 'e';
  return dl_itoa(exponent, buf); // exponent shouldn't be longer than 4 chars
                                 // dl_itoa adds the null-termination
}
