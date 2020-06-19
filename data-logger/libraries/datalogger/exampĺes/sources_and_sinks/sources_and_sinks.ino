#include <Datalogger.h> // <- set DL_DEBUG to 0 inside the Datalogger.h file
                        //    in order to reduce RAM usage

/* Reads from: RTC, analog sensor (turbidity originally, but any will do)
 * Writes to:  Serial, Screen, SD
 */

char date_buf[20]; // strlen("YYYY-MM-DD hh:mm:ss") is 19

/*****************
 * Sources setup
 */

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

    dl_formatdate(date_buf, y, m, d, hh, mm, ss);
    *(uint64_t *)output_ptr = unixtime(y, m, d, hh, mm, ss););

const I2CSource rtc =
    I2CSource(0x68 /* ds3231 address */, 0 /* request command is null byte*/,
              7 /* response size */, rtc_to_unix /* callback */, T_U64);
uint64_t rtc_val; // asocciated read variable

const AnalogSource turb =
    AnalogSource(A1 /* pin */, NULL /* no callback  */, NULL);
int turb_val; // associated read variable

// List of sources: rtc, turb

/*****************
 * Sinks setup
 */

// Serial
dl_type_t sersink_input_types[] = {T_INT, T_U64, T_CHAR_P};
const SerialSink sersink(sersink_input_types, 3, Serial);

// Screen
// In order to use ScreenSink, a write_str implementation must be provided
// u8x8 calls can be made to `scs_driver` object.
int ScreenSink::write_str(char **str_inputs) {
  // str_inputs are in the same order as its corresponding datatypes
  // from `sersink_input_types`
  scs_driver.drawString(0, 0, "Last measurement");
  scs_driver.setCursor(1, 1);
  scs_driver.print(str_inputs[0]);
  scs_driver.print(" "); // clear ebbing digits
  scs_driver.drawString(0, 3, "at");

  // Separate date and hour from str_inputs[1] as screen width (16) is too
  // short to accomodate full timestamp (length 19)
  char *date_part = dl_strtok(str_inputs[1], ' ');
  scs_driver.drawString(1, 4, date_part);
  char *time_part = dl_strtok(NULL, ' ');
  scs_driver.drawString(1, 5, time_part);

  return 1;
}

dl_type_t screen_input_types[] = {T_INT, T_CHAR_P};
const ScreenSink screen(screen_input_types, 2);

// CSV (SD)
dl_type_t csv_input_types[] = {T_U64, T_INT, T_FLOAT};
const CSVSink csv(csv_input_types, /* # of types */ 3, /* chip select pin */ 10,
                  /* csv header */ "timestamp,turbidity,dummyfloat",
                  /* filename prefix */ "data");

// List of sinks: sersink, screen, csv

/*****************
 * Routines
 */

void read_data() {
  turb.read(&turb_val);
  rtc.read(&rtc_val); // also sets date_buf
}

void printer_pipe() {
  // read_data must be called before

  // Output
  sersink.write(3, turb_val, rtc_val, date_buf);
  csv.write(3, rtc_val, turb_val, -0.00324);
  // must be called last, as screen.write explodes date_buf
  screen.write(2, turb_val, date_buf);
}

void setup() {
  Serial.begin(9600);

  Wire.begin();   // I2C
  screen.begin(); // u8x8
  csv.begin();
}

void loop() {
  read_data();
  printer_pipe();
  Serial.println(F("\n--------------------------------"));
  delay(2000);
}
