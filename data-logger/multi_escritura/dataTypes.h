typedef struct sensor_reading {
    float val;
    uint32_t ts;
    uint8_t sensor;
} reading_type;

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 2)/sizeof(reading_type);
//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 2 - DATA_DIM*sizeof(reading_type);

typedef struct data_block { // Greiman
    uint16_t count; // Number of readings on block
    reading_type data[DATA_DIM]; // Readings
    uint8_t fill[FILL_DIM]; // Bytes to fill 512 bytes
} block_type;