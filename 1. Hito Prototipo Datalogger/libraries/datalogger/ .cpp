#include "Datalogger.h"
#include <Wire.h>

// Global union variable to store dereferentiations.
static dl_multi_t deref_var;

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
      assert((F("Error: Callback not set. Pointer is not big enough to hold "
                "value."),
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

// I2CSource

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

// AnalogSource

void *AnalogSource::read_raw() { return (void *)analogRead(analog_pin); }

/****************************** Sink ******************************/
int Sink::write(uint8_t n_args, ...) {
#if DL_DEBUG
  assert(n_args == snk_n_inputs);
#endif
  va_list inputs;
  va_start(inputs, n_args);
  int result = vwrite(inputs);
  va_end(inputs);

  return result;
}

int Sink::vwrite(va_list inputs) {
  char **strings_to_write = malloc(snk_n_inputs * sizeof(char *));
  char **strings_to_free = malloc(snk_n_inputs * sizeof(char *));

  // Allocate only the needed memory to store each variable's type
  // in a string.
  for (uint8_t i = 0; i < snk_n_inputs; i++) {
    dl_type_t input_type = snk_input_types[i];
    switch (input_type) {
    case T_CHAR_P:
      // Serial.println("vwrite: T_CHAR_P");
      strings_to_write[i] = va_arg(inputs, char *);
      strings_to_free[i] = NULL;
      break;
    case T_INT:
      // Serial.println("vwrite: T_INT");
      strings_to_write[i] = calloc(MAX_STR_LEN_INT, 1);
      strings_to_free[i] = strings_to_write[i];
      dl_itoa(va_arg(inputs, int), strings_to_write[i]);
      break;
    case T_U64:
      // Serial.println("vwrite: T_U64");
      strings_to_write[i] = calloc(MAX_STR_LEN_U64, 1);
      strings_to_free[i] = strings_to_write[i];
      dl_u64toa(va_arg(inputs, uint64_t), strings_to_write[i]);
      break;
    case T_FLOAT:
      // Serial.println("vwrite: T_FLOAT");
      strings_to_write[i] = calloc(MAX_STR_LEN_FLOAT, 1);
      strings_to_free[i] = strings_to_write[i];
      // https://stackoverflow.com/a/11270603
      dl_floattosci((float)va_arg(inputs, double), strings_to_write[i]);
      break;
    default:
#if DL_DEBUG
      assert(
          (F("Error: Sink::write lacks this dl_type_t in its switch"), false));
#endif
      break;
    }
  }

  int retval = write_str(strings_to_write);

  // Deallocate memory, only if we previously allocated it here.
  for (uint8_t i = 0; i < snk_n_inputs; i++) {
    if (strings_to_free[i] != NULL)
      free(strings_to_free[i]);
  }

  // Deallocate buffer storers
  free(strings_to_free);
  free(strings_to_write);

  return retval;
}

// Each Sink type implements its own `write_str` method.

// SerialSink

int SerialSink::write_str(char **str_inputs) {
  for (uint8_t i = 0; i < snk_n_inputs - 1; i++) {
    sersnk_serial.print(str_inputs[i]);
    sersnk_serial.print(F(", "));
  }
  sersnk_serial.println(str_inputs[snk_n_inputs - 1]);
  return 1;
}

// ScreenSink

// When using ScreenSink, you must provide your own `write_str` implementation
// before calling its constructor.
// int ScreenSink::write_str(char **str_inputs) { [implementation using
// scs_driver] }

void ScreenSink::begin() {
  scs_driver.begin();
  scs_driver.setFont(u8x8_font_chroma48medium8_r);
  Serial.println(F("ScreenSink loaded"));
}

// CSVSink

void CSVSink::begin() {
  if (!csvsnk_sddriver.begin(csvsnk_cspin, SD_SCK_MHZ(50))) {
    csvsnk_sddriver.initErrorHalt();
  }

  int prefix_len = dl_strlen(csvsnk_prefix); // strlen("data")
  memcpy(csvsnk_filename, csvsnk_prefix, prefix_len);
  memcpy(csvsnk_filename + prefix_len, "00.csv", 6); // "data00.csv"
  csvsnk_filename[prefix_len + 6] = '\0';

  while (csvsnk_sddriver.exists(csvsnk_filename)) {
    if (csvsnk_filename[prefix_len + 1] != '9') {
      csvsnk_filename[prefix_len + 1]++;
    } else if (csvsnk_filename[prefix_len] != '9') {
      csvsnk_filename[prefix_len + 1] = '0';
      csvsnk_filename[prefix_len]++;
    } else {
#if DL_DEBUG
      assert((F("Can't create file name"), false));
#endif
    }
  }
  if (!csvsnk_filedriver.open(csvsnk_filename, O_WRONLY | O_CREAT | O_EXCL)) {
#if DL_DEBUG
    //    ERROR("csvsnk_filedriver.open");
    assert((F("open"), false));
#endif
  }

  // write header
  csvsnk_filedriver.println(csvsnk_header);
  Serial.print(F("CSVSink loaded: '"));
  Serial.print(csvsnk_filename);
  Serial.println(F("' selected as filename"));
}

int CSVSink::write_str(char **str_inputs) {
  for (uint8_t i = 0; i < snk_n_inputs - 1; i++) {
    csvsnk_filedriver.print(str_inputs[i]);
    csvsnk_filedriver.write(',');
  }
  csvsnk_filedriver.println(str_inputs[snk_n_inputs - 1]);
  csvsnk_filedriver.flush();
  return 1;
}

/* **************************** Datalogger **************************** */

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

int Datalogger::write_to(uint8_t sink_index, ...) {
  if (sink_index >= dlg_n_sinks) // Sink doesn't exist
    return 0;

  Sink *snk = dlg_sinks[sink_index];

  va_list inputs;
  va_start(inputs, sink_index);
  int retval = snk->vwrite(inputs);
  va_end(inputs);

  return retval;
}

void *Datalogger::read(uint8_t source_index, void *output_ptr,
                       dl_type_t output_as) {
  if (source_index >= dlg_n_sources)
    return (void *)-1; // Source doesn't exist
  Source *src = dlg_sources[source_index];
  return src ? src->read(output_ptr, output_as) : (void *)-1;
}

void *Datalogger::read(uint8_t source_index, void *output_ptr) {
  if (source_index >= dlg_n_sources)
    return (void *)-1; // Source doesn't exist
  Source *src = dlg_sources[source_index];
  return src ? src->read(output_ptr) : (void *)-1;
}

#if DL_DEBUG
// Prints the name and Source type (dl_source_t) of each Datalogger source.
void Datalogger::list_sources() {
  Serial.println(F("Sources list:"));
  for (uint8_t i = 0; i < dlg_n_sources; i++) {
    Source *src = dlg_sources[i];
    Serial.print(i);
    Serial.print(") ");
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
  for (uint8_t i = 0; i < dlg_n_sources; i++) {
    Source *src = dlg_sources[i];
    dl_type_t output_type = src->src_callback_output_type
                                ? src->src_callback_output_type
                                : src->src_raw_output_type;
    Serial.print(F("Source "));
    Serial.print(i);
    Serial.print(F(") "));
    switch (output_type) {
    case T_U64:
      src->read(&deref_var.T_U64, T_U64);
      print64(deref_var.T_U64);
      break;
    case T_INT:
      src->read(&deref_var.T_INT, T_INT);
      Serial.println(deref_var.T_INT);
      break;
    default:
      Serial.println("print_read_all lacks this dl_type_t case in its switch!");
    }
  }
}
#endif // DL_DEBUG

#if DL_DEBUG
// https://gist.github.com/jlesech/3089916
// handle diagnostic informations given by assertion and abort program
// execution:
void __assert(const char *__func, const char *__file, int __lineno,
              const char *__sexp) {
  // transmit diagnostic informations through serial link.
  Serial.print(F("Assertion error: "));
  Serial.println(__func);
  Serial.println(__file);
  Serial.println(__lineno, DEC);
  Serial.println(__sexp);
  Serial.flush();
  // abort program execution.
  abort();
}
#endif // DL_DEBUG
