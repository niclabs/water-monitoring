#include "Datalogger.h"
#include <Wire.h>

/*
 * Reads from `read_raw` (implemented by descendant classes), and passes
 * its output to the defined in the constructor. If `src_callback` is NULL,
 * interprets its value as if it were of the `output_as` type, and returns
 * it as such. */
void *Source::read(void *output_ptr, dl_type_t output_as) {
  void *raw = read_raw();
  if (src_callback == NULL) {
    switch (output_as) {
    case T_CHAR:
      *(char *)output_ptr = raw;
      break;
    case T_INT:
      *(int *)output_ptr = raw;
      break;
    case T_U16:
      *(uint16_t *)output_ptr = raw;
      break;
    case T_CHAR_P:
      *(char **)output_ptr = raw;
      break;
    case T_U64_P:
      *(uint64_t *)output_ptr = raw;
      break;
    // a pointer has only 16 bits
    case T_FLOAT: // <- 32 bits
    case T_U32:
    case T_U64:
    default:
#if DL_DEBUG
      assert(
          ("Error: Callback not set. Pointer is not big enough to hold value.",
           false));
#endif
      *(char *)output_ptr = -1;
    }
    return;
  }

  // output by pointer (doesn't return)
  src_callback(raw, src_raw_output_type, output_ptr);
}

void *Source::read(void *output_ptr) {
  if (src_callback_output_type == NULL)
    return Source::read(output_ptr, src_raw_output_type);
  return Source::read(output_ptr, src_callback_output_type);
}

// Each Source type implements its own `read_raw` method.

void *I2CSource::read_raw() {
  Wire.beginTransmission(i2c_address);
  Wire.write(i2c_request_cmd);
  Wire.endTransmission();

  memset(i2c_rx_buffer, '\0', i2c_response_size);

  Wire.requestFrom(i2c_address, i2c_response_size);
  for (uint8_t i = 0; Wire.available() && i < i2c_response_size; i++) {
    i2c_rx_buffer[i] = Wire.read();
  }

  return i2c_rx_buffer;
}

void *AnalogSource::read_raw() { return (void *)analogRead(analog_pin); }

// Called from the main `setup()`
void Datalogger::begin() {
  /*
   * I think these "exploratory" routines should be defined at compilation
   * time, settting just the necessary initializations */

  // Check if Wire.h must be initialized (I2C)
  for (uint8_t i = 0; i < dlg_n_sources; i++) {
    Source *src = dlg_sources[i];
    if (src->src_type == SRC_I2C) {
      Wire.begin();
      break;
    }
  }
}

#if DL_DEBUG
// Prints the name and Source type (dl_source_t) of each Datalogger source.
void Datalogger::list_sources() {
  Serial.println(F("Sources list:"));
  for (uint8_t i = 0; i < dlg_n_sources; i++) {
    Source *src = dlg_sources[i];
    Serial.print(i);
    Serial.print(") ");
    Serial.print(src->src_name);
    Serial.print(F(": "));
    switch (src->src_type) {
    case SRC_NONE:
      Serial.println(F("SRC_NONE"));
      break;
    case SRC_I2C:
      Serial.println(F("SRC_I2C"));
      break;
    case SRC_ANALOG:
      Serial.println(F("SRC_ANALOG"));
    }
  }
}

// Reads data from all the Datalogger Sources, and prints their value
void Datalogger::print_read_all() {
  uint64_t var64;
  int varint;
  for (uint8_t i = 0; i < dlg_n_sources; i++) {
    Source *src = dlg_sources[i];
    dl_type_t output_type = src->src_callback_output_type
                                ? src->src_callback_output_type
                                : src->src_raw_output_type;
    Serial.print("Source ");
    Serial.print(i);
    Serial.print(F(") "));
    Serial.print(src->src_name);
    Serial.print(": ");
    switch (output_type) {
    case T_U64:
      src->read(&var64, T_U64);
      print64(var64);
      break;
    case T_INT:
      src->read(&varint, T_INT);
      Serial.println(varint);
      break;
    default:
      Serial.println("print_read_all lacks this dl_type_t case in its switch!");
    }
  }
}

// https://gist.github.com/jlesech/3089916
// handle diagnostic informations given by assertion and abort program
// execution:
void __assert(const char *__func, const char *__file, int __lineno,
              const char *__sexp) {
  // transmit diagnostic informations through serial link.
  Serial.print("Assertion error: ");
  Serial.println(__func);
  Serial.println(__file);
  Serial.println(__lineno, DEC);
  Serial.println(__sexp);
  Serial.flush();
  // abort program execution.
  abort();
}
#endif // DL_DEBUG
