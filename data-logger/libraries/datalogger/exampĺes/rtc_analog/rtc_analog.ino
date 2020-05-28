#include <Datalogger.h>

String date_str = "";

I2C_CALLBACK_FUNCTION(
    /*name*/ rtc_to_unix,
    /*input*/ i2c_void_buffer,
    /*output*/ output_ptr,
    /* callback body */

    // i2c_void_buffer is of (void*) type, it must be casted before using
    char *i2c_buffer = (char *)i2c_void_buffer;
    uint8_t ss = bcd2bin(i2c_buffer[0] & 0x7F);
    uint8_t mm = bcd2bin(i2c_buffer[1]); uint8_t hh = bcd2bin(i2c_buffer[2]);
    uint8_t d = bcd2bin(i2c_buffer[4]); uint8_t m = bcd2bin(i2c_buffer[5]);
    uint16_t y = bcd2bin(i2c_buffer[6]) + 2000;

    // "YYYY-MM-DD hh:mm:ss"
    date_str = String(y, DEC);
    date_str += m < 10 ? "-0" : "-"; date_str += String(m, DEC);
    date_str += d < 10 ? "-0" : "-"; date_str += String(d, DEC);
    date_str += hh < 10 ? " 0" : " "; date_str += String(hh, DEC);
    date_str += mm < 10 ? ":0" : ":"; date_str += String(mm, DEC);
    date_str += ss < 10 ? ":0" : ":"; date_str += String(ss, DEC);

    *(uint64_t *)output_ptr = unixtime(y, m, d, hh, mm, ss););

I2CSource rtc =
    I2CSource("rtc" /* name */, 0x68 /* ds3231 address */,
              0 /* request command is null byte*/, 7 /* response size */,
              rtc_to_unix /* callback */, T_U64);

AnalogSource turb = AnalogSource("turb" /* name */, A1 /* pin */,
                                 NULL /* no callback  */, NULL);

Source *sources[] = {&rtc, &turb};
Datalogger dl(sources, 2 /* # of sources */);

void setup() {
  Serial.begin(9600);
  date_str.reserve(19);

  dl.begin(); // initializes necessary resources (e. g. Wire)
  dl.list_sources();
}

void loop() {
  dl.print_read_all();
  Serial.println("fecha rtc: " + date_str);
  Serial.println("--------------");
  delay(2000);
}
