#include "SensorPayload.h"

int8_t getSign(float val) {
    return val > 0 ? 1 : -1;
}

// ---------- Public methods ----------------

SensorPayload::SensorPayload(uint16_t buf_size) {
    _buffer_size = buf_size;
    _pointer = 0;
    _buffer = new uint8_t[buf_size+2]; //Add two bytes for Checksum
}

SensorPayload::~SensorPayload() {
    delete _buffer;
}

uint8_t *SensorPayload::getBuffer() {
    return _buffer;
}

uint16_t SensorPayload::getCurrentSize() {
    return _pointer;
}

void SensorPayload::resetBuffer() {
    _pointer = 0;
}

uint16_t SensorPayload::availableSize() {
    if(_pointer > _buffer_size){
        return 0;
    }
    return _buffer_size - _pointer;
}

void SensorPayload::addChecksum() {
    if(_pointer < 1) {
        return;
    }
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;
    for(uint8_t i=0; i<_pointer; ++i) {
        sum1 += _buffer[i];
        sum2 += sum1;
    }
    sum1 %= 255;
    sum2 %= 255;
    _buffer[_pointer] = 255 - ((sum1 + sum2) % 255);
    _buffer[_pointer+1] = 255 - ((sum1 + _buffer[_pointer]) % 255);
    _pointer += 2;
}

int SensorPayload::encodeReadings(block_type *block, uint8_t next_to_encode, uint8_t encode_mode) {
    if(next_to_encode < block->count) {
        switch(encode_mode) {
            case(REP_MODE):
                return encodeRepeatedReadings(block, next_to_encode);
            case(DIF_MODE):
                return encodeDiferentialReadings(block, next_to_encode);
            default:
                return encodeBaseReadings(block, next_to_encode);
        }
    }
    return 0;
}

// ---------- Private methods ----------------

int SensorPayload::encodeBaseReadings(block_type *block, uint8_t next_to_encode) {
    int n_encoded = 0;
    uint16_t available = availableSize();
    while(available > 6 && next_to_encode < block->count) {
        encodeBaseSensingUnit(block->data[next_to_encode++]);
        ++n_encoded;
        available = availableSize();
    }
    return n_encoded;
}

void SensorPayload::encodeBaseSensingUnit(reading_type reading, uint8_t header) {
    _buffer[_pointer++] = reading.sensor | header;
    _buffer[_pointer++] = reading.ts >> 24;
    _buffer[_pointer++] = reading.ts >> 16;
    _buffer[_pointer++] = reading.ts >> 8;
    _buffer[_pointer++] = reading.ts;
    uint16_t store_value;
    if(reading.val < 0) {
        store_value = (uint16_t)(reading.val*-10);
        _buffer[_pointer++] = 128 | (store_value >> 8);
    }
    else {
        store_value = (uint16_t)(reading.val*10);
        _buffer[_pointer++] = 127 & (store_value >> 8);
    }
    _buffer[_pointer++] = store_value;
}

void SensorPayload::encodeBaseSensingUnit(reading_type reading, int8_t sign, uint16_t val, uint8_t header) {
    _buffer[_pointer++] = reading.sensor | header;
    _buffer[_pointer++] = reading.ts >> 24;
    _buffer[_pointer++] = reading.ts >> 16;
    _buffer[_pointer++] = reading.ts >> 8;
    _buffer[_pointer++] = reading.ts;
    if(sign < 0) {
        _buffer[_pointer++] = 128 | (val >> 8);
    }
    else {
        _buffer[_pointer++] = 127 & (val >> 8);
    }
    _buffer[_pointer++] = val;
}


int SensorPayload::encodeRepeatedReadings(block_type *block, uint8_t next_to_encode) {
    uint16_t available;
    int n_encoded = 0;
    while(next_to_encode < block->count) {
        available = availableSize();
        if(available < 7) { // No space available
            return n_encoded;
        }
        if(available < 14) { // Only 7 bytes available
            encodeBaseSensingUnit(block->data[next_to_encode]);
            return n_encoded + 1;
        }
        if(block->count - next_to_encode < 3) { // There are less than 3 readings to encode
            encodeBaseSensingUnit(block->data[next_to_encode++]);
            if(next_to_encode < block->count) {
                encodeBaseSensingUnit(block->data[next_to_encode]);
                return n_encoded + 2;
            }
            return n_encoded + 1;
        } // At this point there are at least 3 readings to encode and 14 bytes of encoding space 
        int8_t first_sign = getSign(block->data[next_to_encode].val);
        uint16_t first_val = (uint16_t)(block->data[next_to_encode].val*first_sign*10);
        int8_t next_sign = getSign(block->data[next_to_encode+1].val);
        uint16_t next_val = (uint16_t)(block->data[next_to_encode+1].val*next_sign*10);
        if(first_sign != next_sign || first_val != next_val) {
            encodeBaseSensingUnit(block->data[next_to_encode], first_sign, first_val);
            ++next_to_encode;
            ++n_encoded;
            continue;
        }
        next_sign = getSign(block->data[next_to_encode+2].val);
        next_val = (uint16_t)(block->data[next_to_encode+2].val*next_sign*10);
        if(first_sign != next_sign || first_val != next_val) {
            encodeBaseSensingUnit(block->data[next_to_encode], first_sign, first_val);
            encodeBaseSensingUnit(block->data[next_to_encode+1], first_sign, first_val);
            next_to_encode += 2;
            n_encoded += 2;
            continue;
        }
        uint16_t time_interval = (uint16_t)(block->data[next_to_encode+1].ts - block->data[next_to_encode].ts);
        uint16_t num_of_values = 2;
        encodeBaseSensingUnit(block->data[next_to_encode], first_sign, first_val, 176); // BaseSensingUnit with Repeated header
        next_to_encode += 3;
        n_encoded += 3;
        while(next_to_encode < block->count) {
            next_sign = getSign(block->data[next_to_encode].val);
            next_val = (uint16_t)(block->data[next_to_encode].val*next_sign*10);
            if(first_sign != next_sign || first_val != next_val) {
                break;   
            }
            ++next_to_encode;
            ++n_encoded;
            ++num_of_values;
        }
        encodeRepeatedSensingUnit(block->data[next_to_encode-1].sensor, time_interval, num_of_values);
    }
    return n_encoded;
}

void SensorPayload::encodeRepeatedSensingUnit(uint8_t sensor, uint16_t time_interval, uint16_t num_of_values) {
    _buffer[_pointer++] = sensor | 176;
    _buffer[_pointer++] = time_interval >> 8;
    _buffer[_pointer++] = time_interval;
    _buffer[_pointer++] = 0;
    _buffer[_pointer++] = 0;
    _buffer[_pointer++] = num_of_values >> 8;
    _buffer[_pointer++] = num_of_values;
}

int SensorPayload::encodeDiferentialReadings(block_type *block, uint8_t next_to_encode) {
    uint16_t available;
    int n_encoded = 0;
    while(next_to_encode < block->count) {
        available = availableSize();
        if(available < 7) {
            return n_encoded;
        }
        if(available < 14) {
            encodeBaseSensingUnit(block->data[next_to_encode]);
            return n_encoded + 1;
        }
        if(block->count - next_to_encode < 3) { // There are less than 3 readings to encode
            encodeBaseSensingUnit(block->data[next_to_encode++]);
            if(next_to_encode < block->count) {
                encodeBaseSensingUnit(block->data[next_to_encode]);
                return n_encoded + 2;
            }
            return n_encoded + 1;
        }
        float first_val_dif = block->data[next_to_encode+1].val - block->data[next_to_encode].val;
        if(first_val_dif < 0 && first_val_dif <= -12.8) {
            encodeBaseSensingUnit(block->data[next_to_encode++]);
            ++n_encoded;
            continue;
        }
        else if(first_val_dif >= 12.8) {
            encodeBaseSensingUnit(block->data[next_to_encode++]);
            ++n_encoded;
            continue;
        }
        float second_val_dif = block->data[next_to_encode+2].val - block->data[next_to_encode].val;
        if(second_val_dif < 0 && second_val_dif <= -12.8) {
            encodeBaseSensingUnit(block->data[next_to_encode++]);
            ++n_encoded;
            continue;
        }
        else if(second_val_dif >= 12.8) {
            encodeBaseSensingUnit(block->data[next_to_encode++]);
            ++n_encoded;
            continue;
        }
        uint16_t first_ts_dif = block->data[next_to_encode+1].ts - block->data[next_to_encode].ts;
        uint16_t second_ts_dif = block->data[next_to_encode+2].ts - block->data[next_to_encode].ts;
        encodeBaseSensingUnit(block->data[next_to_encode], 112); // Base Sensing Unit with Diferential Header
        encodeDiferentialSensingUnit(block->data[next_to_encode].sensor, first_ts_dif, first_val_dif, second_ts_dif, second_val_dif);
        uint8_t ref_index = next_to_encode;
        n_encoded += 3;
        next_to_encode += 3;
        available = availableSize();
        while(available > 6 && next_to_encode + 1 < block->count) {
            first_val_dif = block->data[next_to_encode].val - block->data[ref_index].val;
            if(first_val_dif < 0 && first_val_dif <= -12.8) {
                break;
            }
            else if(first_val_dif >= 12.8) {
                break;
            }
            second_val_dif = block->data[next_to_encode+1].val - block->data[ref_index].val;
            if(second_val_dif < 0 && second_val_dif <= -12.8) {
                break;
            }
            else if(second_val_dif >= 12.8) {
                break;
            }
            first_ts_dif = block->data[next_to_encode].ts - block->data[ref_index].ts;
            second_ts_dif = block->data[next_to_encode+1].ts - block->data[ref_index].ts;
            encodeDiferentialSensingUnit(block->data[ref_index].sensor, first_ts_dif, first_val_dif, second_ts_dif, second_val_dif);
            n_encoded += 2;
            next_to_encode += 2;
            available = availableSize();
        }
    }
    return n_encoded;
}

void SensorPayload::encodeDiferentialSensingUnit(uint8_t sensor,  uint16_t first_ts, float first_val, uint16_t second_ts, float second_val) {
    int8_t f_sign = getSign(first_val);
    int8_t s_sign = getSign(second_val);
    uint8_t f_val = (uint8_t) (f_sign * first_val * 10);
    uint8_t s_val = (uint8_t) (s_sign * second_val * 10);
    _buffer[_pointer++] = sensor | 112;
    _buffer[_pointer++] = first_ts >> 8;
    _buffer[_pointer++] = first_ts;
    if(f_sign < 0) {
        _buffer[_pointer++] = 128 | f_val;
    }
    else {
        _buffer[_pointer++] = 127 & f_val;
    }
    _buffer[_pointer++] = second_ts >> 8;
    _buffer[_pointer++] = second_ts;
    if(s_sign < 0) {
        _buffer[_pointer++] = 128 | s_val;
    }
    else {
        _buffer[_pointer++] = 127 & s_val;
    }
}