#ifndef DATALOGGER_H
#define DATALOGGER_H

/* Datalogger.h defines 2 kinds of objects:
 * - Sources of data, from which information can be read.
 * - Sinks of data, to which information can be written.
 *
 * Already implemented classes provide connections to Analog and I2C sources,
 * but thanks to its explicit type-passing system, can extend it to many
 * other protocols, defining processing methods (`read_raw`) and
 * post-processing Callback functions.
 *
 * Sinks work in two steps:
 * - An invocation of `write` (always implemented by main Sink class)
 *   passing arbitrary typed data converts it to an array of strings.
 * - Each Sink type implements its own method `write_str`, which takes
 *   the array of strings of data, and handles it any way it wants.
 * Due to code simplification, Sinks have no callbacks.
 * ***************************************************************************/

#define DL_DEBUG                                                               \
  0 // Toggles CALLBACK_FUNCTION assertions for type checking
    // and other debugging-related print functions

#include <Arduino.h>
#include <Wire.h>
#include <stdarg.h>

#if DL_DEBUG
#define __ASSERT_USE_STDERR // Must be set before assert.h
#endif                      // DL_DEBUG

#include "utils.h"
#include <assert.h>

// Allowed types (for dereferencing void pointers)
enum dl_type_t {
  T_NONE = NULL,
  T_CHAR,
  T_INT,
  T_FLOAT, // float
  T_U16,
  T_U32,
  T_U64,
  T_CHAR_P, // char*
  T_U64_P,  // uint64_t*
};

// Length of max number representable with N bits (considering sign)
#define MAX_LEN_8_BITS 5   // "-128"
#define MAX_LEN_16_BITS 7  // "-32768"
#define MAX_LEN_32_BITS 12 // "-2147483648"
#define MAX_LEN_64_BITS 21 // "-9223372036854775808"

// Max string length when converting types to string
#define MAX_STR_LEN_INT MAX_LEN_16_BITS
#define MAX_STR_LEN_U16 MAX_LEN_16_BITS
#define MAX_STR_LEN_U32 MAX_LEN_32_BITS
#define MAX_STR_LEN_U64 MAX_LEN_64_BITS
#define MAX_STR_LEN_FLOAT 14 // see `dl_floattosci` in utils.cpp

// Data type to inform void pointer derreferenciations
typedef union available_types {
  char T_CHAR;
  int T_INT;
  float T_FLOAT;
  uint16_t T_U16;
  uint32_t T_U32;
  uint64_t T_U64;
} dl_multi_t;

// Types of Sources
enum dl_source_t { SRC_NONE, SRC_I2C, SRC_ANALOG };

// Callback function type
typedef int *(*dl_cback_t)(void *, dl_type_t, void *);

// Inserts primitive typechecking inside callback functions
#if DL_DEBUG
#define CALLBACK_DEBUG_LINE(expected_input_type)                               \
  assert(real_input_type == expected_input_type);
#else
#define CALLBACK_DEBUG_LINE(expected_input_type)
#endif // DL_DEBUG

#define CALLBACK_FUNCTION(name, expected_input_type, input_variable,           \
                          output_pointer, ...)                                 \
  void name(void *input_variable, dl_type_t real_input_type,                   \
            void *output_pointer) {                                            \
    CALLBACK_DEBUG_LINE(expected_input_type)                                   \
    __VA_ARGS__                                                                \
  }

/*##############################################################################
 *
 *                                  SOURCES
 *
 * ########################################################################## */

/*
 * Sources have:
 * - a return type (a dl_type_t) for its `read_raw` function
 * - optionally, a dl_cback_t callback function, which processes its input from
 * `read_raw` and passes it as output in the `read` function.
 * - optionally, a return type for the callback function
 *
 * By default, Sources are initialized with a SRC_NONE source type. Subclasses
 * may change it.
 * */
class Source {
public:
  Source(dl_type_t raw_output_type, dl_cback_t callback = NULL,
         dl_type_t callback_output_type = NULL)
      : src_raw_output_type{raw_output_type}, src_callback{callback},
        src_callback_output_type{callback_output_type} {
    src_type = SRC_NONE;
  }

  virtual void *read_raw();
  void *read(void *output_ptr, dl_type_t output_as);
  void *read(void *output_ptr);

  dl_source_t src_type;
  dl_type_t src_raw_output_type;
  dl_type_t src_callback_output_type;

private:
  dl_cback_t src_callback;
};

/*
 * I2C Sources have particularly:
 * - a 1 byte I2C address
 * - a 1 byte request command (issued to ask for data)
 * - a response size, which corresponds to the allocated rx buffer size
 * All I2C callbacks must accept a char* (T_CHAR_P) as its input, which
 * corresponds to the received message buffer. */

#define I2C_CALLBACK_FUNCTION(name, input_variable, output_pointer, ...)       \
  CALLBACK_FUNCTION(name, T_CHAR_P, input_variable, output_pointer, __VA_ARGS__)

class I2CSource : public Source {
public:
  I2CSource(uint8_t address, const char request_cmd, int response_size,
            dl_cback_t callback, dl_type_t callback_output_type)
      : Source(T_CHAR_P, callback, callback_output_type), i2c_address{address},
        i2c_request_cmd{request_cmd}, i2c_response_size{response_size} {
    i2c_rx_buffer = calloc(response_size, 1);
    src_type = SRC_I2C;
  }

  ~I2CSource() { free(i2c_rx_buffer); }

  void *read_raw();
  using Source::read;

  char *i2c_rx_buffer;

private:
  uint8_t i2c_address;
  uint8_t i2c_response_size;
  char i2c_request_cmd;
};

/*
 * Analog Sources have particularly:
 * - an integer pin from which to read its data
 * All Analog callbacks must accept an int (T_INT) as its input. */

#define ANALOG_CALLBACK_FUNCTION(name, input_variable, output_pointer, ...)    \
  CALLBACK_FUNCTION(name, T_INT, input_variable, output_pointer, __VA_ARGS__)

class AnalogSource : public Source {
public:
  AnalogSource(int pin, dl_cback_t callback, dl_type_t callback_output_type)
      : Source(T_INT, callback, callback_output_type), analog_pin{pin} {
    src_type = SRC_ANALOG;
  }

  void *read_raw();

private:
  int analog_pin;
};

/*##############################################################################
 *
 *                                   SINKS
 *
 * ########################################################################## */

/*
 * Sinks have:
 * - an array describing the types of the inputs (dl_type_t) for its `write`
 * function
 * - the number of inputs accepted in `write`
 *
 * In order to instantiate a Sink object, its
 *                  int <ClassName>::write_str(char **str_inputs)
 * method must be implemented beforehand.
 * */

class Sink {
public:
  Sink(dl_type_t *input_types, uint8_t n_inputs)
      : snk_input_types{input_types}, snk_n_inputs{n_inputs} {}

  int write(uint8_t n_args, ...);
  int vwrite(va_list inputs); // like printf and vprintf
  virtual int write_str(char **str_inputs);

  dl_type_t *snk_input_types;
  uint8_t snk_n_inputs;
};

/*
 * Serial Sinks have particularly:
 * - a HardwareSerial reference (usually by passing `Serial` you're fine)
 *
 * SerialSink already provides its own `write_str` implementation.
 * */

class SerialSink : public Sink {
public:
  SerialSink(dl_type_t *input_types, uint8_t n_inputs, HardwareSerial &serial)
      : Sink(input_types, n_inputs), sersnk_serial{serial} {}
  int write_str(char **str_inputs);

private:
  HardwareSerial &sersnk_serial;
};

/*
 * Screen Sinks instantiate:
 * - a U8x8 screen driver.
 *
 * Its `begin` method must be called at setup.
 * Its `write_str` method must be implemented (using the scs_driver reference)
 * */

#include <U8g2lib.h>
class ScreenSink : public Sink {
public:
  ScreenSink(dl_type_t *input_types, uint8_t n_inputs)
      : Sink(input_types, n_inputs), scs_driver(U8X8_PIN_NONE) {}
  int write_str(char **str_inputs);
  void begin();

private:
  U8X8_SSD1306_128X64_NONAME_HW_I2C scs_driver;
};

/*
 * CSV Sinks have particularly:
 * - an integer Chip Select pin
 * - a string corresponding to the CSV header
 * - a prefix string for the name of the CSV file. MUST NOT BE OVER 6 CHARACTERS
 * LONG! CSV Sinks instantiate:
 * - a SdFat driver.
 * - a SdFat driver.
 *
 * CSVSink already provides its own `write_str` implementation.
 * */

#include "SdFat.h" // Bill Greiman version
class CSVSink : public Sink {
public:
  CSVSink(dl_type_t *input_types, uint8_t n_inputs, int cs_pin,
          const char *csv_header, const char *filename_prefix)
      : Sink(input_types, n_inputs), csvsnk_cspin{cs_pin},
        csvsnk_header{csv_header}, csvsnk_prefix{filename_prefix} {}
  int write_str(char **str_inputs);
  void begin();

private:
  int csvsnk_cspin;
  char *csvsnk_prefix;
  char csvsnk_filename[13];
  char *csvsnk_header;

  // +653 bytes RAM (1817/2048 B)
  SdFat csvsnk_sddriver;
  SdFile csvsnk_filedriver;
};

/*##############################################################################
 *
 *                                DATALOGGER
 *  Refrain from using it.
 *  Turned out to be either too complicated or a waste of memory.
 *
 * ########################################################################## */

/*
 * A Datalogger has an array of pointers to Sources, and
 *                  an array of pointers to Sinks */
class Datalogger {
public:
  Datalogger(const Source *sources[], uint8_t n_sources, const Sink *sinks[],
             uint8_t n_sinks)
      : dlg_n_sources{n_sources}, dlg_n_sinks{sinks} {
    dlg_sources = sources;
    dlg_sinks = sinks;
  }
  void begin();
  int write_to(uint8_t sink_index, ...);

  void *read(uint8_t source_index, void *output_ptr, dl_type_t output_as);
  void *read(uint8_t source_index, void *output_ptr);

#if DL_DEBUG
  void list_sources();
  void print_read_all();
#endif // DL_DEBUG
private:
  Source **dlg_sources;
  Sink **dlg_sinks;
  uint8_t dlg_n_sources;
  uint8_t dlg_n_sinks;
};

#endif // DATALOGGER_H
