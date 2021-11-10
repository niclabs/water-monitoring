#ifndef SensorPayload_h
#define SensorPayload_h

#if defined(ARDUINO)
#include "Arduino.h"
#endif // defined(ARDUINO)
#include <stdint.h>
#include "dataTypes.h"

#define BASE_MODE 0x00
#define REP_MODE 0x01
#define DIF_MODE 0x02

class SensorPayload {
    public:
        SensorPayload(uint16_t buf_size);
        ~SensorPayload();
        uint8_t *getBuffer();
        uint16_t getCurrentSize();
        void resetBuffer();
        uint16_t availableSize();
        void addChecksum();
        int encodeReadings(block_type *block, uint8_t curr_pos, uint8_t encode_mode);

    private:
        uint8_t *_buffer;
        uint16_t _buffer_size;
        uint16_t _pointer;
        int encodeBaseReadings(block_type *block, uint8_t next_to_encode);
        void encodeBaseSensingUnit(reading_type reading, uint8_t header=240);
        void encodeBaseSensingUnit(reading_type reading, int8_t sign, uint16_t val, uint8_t header=240);
        int encodeRepeatedReadings(block_type *block, uint8_t next_to_encode);
        void encodeRepeatedSensingUnit(uint8_t sensor, uint16_t time_interval, uint16_t num_of_values);
        int encodeDiferentialReadings(block_type *block, uint8_t next_to_encode);
        void encodeDiferentialSensingUnit(uint8_t sensor,  uint16_t first_ts, float first_val, uint16_t second_ts, float second_val);
};

#endif // SensorPayload_h
