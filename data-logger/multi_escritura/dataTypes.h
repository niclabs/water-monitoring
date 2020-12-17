typedef struct sensor_reading {
    float val;
    uint32_t ts;
    uint8_t sensor;
} reading_type;

#define TEMP_DS18B20 0x00
#define TEMP_SHT20 0x01
#define PRES_GRAVITY 0x02
#define PRES_HK1100C 0x03
#define PH_GRAVITY 0x04
#define EC_GRAVITY 0x05
#define TDS_GRAVITY 0x06
#define TURB_GROVE 0x07

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 2)/sizeof(reading_type);
//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 2 - DATA_DIM*sizeof(reading_type);

typedef struct data_block { // Greiman
    uint16_t count; // Number of readings on block
    reading_type data[DATA_DIM]; // Readings
    uint8_t fill[FILL_DIM]; // Bytes to fill 512 bytes
} block_type;
