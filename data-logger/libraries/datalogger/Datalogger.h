#ifndef DATALOGGER_H
#define DATALOGGER_H

#define DL_DEBUG                                                               \
  1 // Toggles CALLBACK_FUNCTION assertions for type checking
    // and other debugging-related print functions

#include <Arduino.h>
#include <Wire.h>

#if DL_DEBUG
#define __ASSERT_USE_STDERR // Must be set before assert.h
#endif                      // DL_DEBUG

#include "utils.h"
#include <assert.h>

// Allowed types (for derreferencing void pointers)
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

/*
 * Sources have:
 * - a constant unique name
 * - a return type (a dl_type_t) for its `read_raw` function
 * - optionally, a dl_cback_t callback function, which processes its input from
 * `read_raw` and passes it as output in the `read` function.
 * - optionally, a return type for the callback function
 *
 * By default, Sources are initialized with a S_NONE source type. Subclasses may
 * change it.
 * */
class Source {
public:
  Source(const char name[], dl_type_t raw_output_type,
         dl_cback_t callback = NULL, dl_type_t callback_output_type = NULL)
      : src_name{name}, src_raw_output_type{raw_output_type},
        src_callback{callback}, src_callback_output_type{callback_output_type} {
    src_type = SRC_NONE;
  }

  virtual void *read_raw();
  void *read(void *output_ptr, dl_type_t output_as);
  void *read(void *output_ptr);

  char *src_name;
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
  I2CSource(const char name[], uint8_t address, const char request_cmd,
            int response_size, dl_cback_t callback,
            dl_type_t callback_output_type)
      : Source(name, T_CHAR_P, callback, callback_output_type),
        i2c_address{address}, i2c_request_cmd{request_cmd}, i2c_response_size{
                                                                response_size} {
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
  AnalogSource(const char name[], int pin, dl_cback_t callback,
               dl_type_t callback_output_type)
      : Source(name, T_INT, callback, callback_output_type), analog_pin{pin} {
    src_type = SRC_ANALOG;
  }

  void *read_raw();

private:
  int analog_pin;
};

/*
 * TODO:
 * - Define Sinks of data (such as Screens or SD), as opposite to Sources
 * - Add events actions (as a response to the push of a button, or a timer
 * interruption).
 *  */

/*
 * A Datalogger has a list of Sources */
class Datalogger {
public:
  Datalogger(const Source *sources[], const uint8_t n_sources)
      : dlg_n_sources{n_sources} {
    dlg_sources = sources;
  }
  void begin();

#if DL_DEBUG
  void list_sources();
  void print_read_all();
#endif // DL_DEBUG
private:
  Source **dlg_sources;
  uint8_t dlg_n_sources;
};

#endif // DATALOGGER_H
