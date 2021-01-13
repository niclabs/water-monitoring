#include <Arduino.h>
#include "SdFat.h" // Bill Greiman
#include <avr/sleep.h>
#include "SensorPayload.h"
#include <SoftwareSerial.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1015.h> // ADC
SoftwareSerial softSerial(8,9);

// ------------------ SensorPayload --------------------
#define SEND_AS_PRINT 0 // If 1, SensorPayload will not be used,
                        // and data will be printed as text
#define BUFFER_SIZE 49
SensorPayload sp(BUFFER_SIZE);

// ---------------------- RTC --------------------------
#define CLOCK_INTERRUPT_PIN 2
#define SENSE_FLAG 0x01
#define SEND_FLAG 0x02
RTC_DS3231 rtc;
uint32_t alarm_time;
volatile uint32_t elapsed_time;
volatile uint8_t alarm_flag;

// ---------------------- DS18B20 ----------------------
// DS18B20 setup variables
#define ONE_WIRE_BUS 5
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// ---------------------- ADC ----------------------
Adafruit_ADS1015 adc;     /* Use this for the 12-bit version */

// --------------------- File configuration variables --------------------------------
#define FILE_BASE_NAME "data"
#define TMP_FILE_NAME FILE_BASE_NAME "###.bin"
// Number of blocks of 512 bytes per file
const uint32_t FILE_BLOCK_COUNT = 2;
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
const uint8_t FILE_NAME_DIM  = BASE_NAME_SIZE + 8;
char binName[FILE_NAME_DIM] = FILE_BASE_NAME "000.bin";
char first_pending_file[3] = {'#', '#', '#'};
uint8_t pending_blocks;
uint8_t next_to_send;

//----------------------- SD configuration variables ---------------------------------
// SD chip select pin.
#define CHIP_SELECT 10
// SD Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
SdFat sd;
SdBaseFile binFile;
block_type *sd_block;
volatile uint16_t written_blocks;

// ----------------------- SoftSerial sending -----------------------
int n_sent, n_to_send;

//------------------------ Sensors configuration variables -----------------------
// SENSING_FREQ_SECS must be less or equal to SENDING_FREQ_SECS
#define SENSING_FREQ_SECS 1
#define SENDING_FREQ_SECS 50
#define N_SENSORS 5
/* Sensors present on the data logger. If there are repeated sensors on the datalogger,
write them multiple times in the array.
For example, if there are 3 DS18B20 sensors, the array must contain the TEMP_DS18B20 value three times.
*/
uint8_t registered_sensors[N_SENSORS] = {
                                        TDS_GRAVITY,
                                        PH_GRAVITY,
                                        TURB_GROVE,
                                        PRES_GRAVITY,
                                        TEMP_DS18B20
                                        };
// Typedef of function to read values from sensors
typedef float (*sensorValueFunction) (void);
/* User defined functions to read data from the registered_sensors array. Must return float and receive 
no parameters
*/

float adcReader(int channel) {
    int16_t reading = adc.readADC_SingleEnded(channel); // return as RAW
    return (float)reading;
}

float tdsValue() { return adcReader(0); }
float phValue() { return adcReader(1); }
float turbValue() { return adcReader(2); }
float presValue() { return adcReader(3); }
float ds18b20value() {
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    return temp;
}

/* Array of pointers to functions to read data from sensors. The order of functions in the array must
follow the same order present in the registered_sensors array.
For example, if registered_sensors = { X, Y, Z }, then the function array must be
sensors_functions = {logDataX, logDataY, logDataZ}
*/

sensorValueFunction sensors_functions[N_SENSORS] = {
                                                   tdsValue,
                                                   phValue,
                                                   turbValue,
                                                   presValue,
                                                   ds18b20value
                                                   };

//--------------------------------- File Functions -----------------------------------------------

void createBinFile() {
    // max number of blocks to erase per erase call
    const uint32_t ERASE_SIZE = 262144L;
    uint32_t bgnBlock, endBlock;
    // Delete old tmp file.
    if (sd.exists(TMP_FILE_NAME)) {
        Serial.println(F("Deleting tmp file " TMP_FILE_NAME));
        if (!sd.remove(TMP_FILE_NAME)) {
            error("Can't remove tmp file");
        }
    }
    // Create new file.
    Serial.println(F("\nCreating new file"));
    binFile.close();
    if (!binFile.createContiguous(TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
        error("createContiguous failed");
    }
    // Get the address of the file on the SD.
    if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
        error("contiguousRange failed");
    }
    // Flash erase all data in the file.
    Serial.println(F("Erasing all data"));
    uint32_t bgnErase = bgnBlock;
    uint32_t endErase;
    while (bgnErase < endBlock) {
        endErase = bgnErase + ERASE_SIZE;
        if (endErase > endBlock) {
            endErase = endBlock;
        }
        if (!sd.card()->erase(bgnErase, endErase)) {
            error("erase failed");
        }
        bgnErase = endErase + 1;
    }
    if (!sd.card()->writeStart(binFile.firstBlock())) {
        error("writeStart failed");
    }
    sd_block->count = 0;
    written_blocks = 0;
}

void renameBinFile() {
    while (sd.exists(binName)) {
        if (binName[BASE_NAME_SIZE + 2] != '9') {
            binName[BASE_NAME_SIZE + 2]++;
        } 
        else if (binName[BASE_NAME_SIZE + 1] != '9') {
            binName[BASE_NAME_SIZE + 2] = '0';
            binName[BASE_NAME_SIZE + 1]++;
        }
        else if (binName[BASE_NAME_SIZE] != '9') {
            binName[BASE_NAME_SIZE + 1] = '0';
            binName[BASE_NAME_SIZE]++;
        }
        else {
            error("Can't create file name");
        }
    }
    if (!binFile.rename(binName)) {
        error("Can't rename file");
    }
    Serial.print(F("File renamed: "));
    Serial.println(binName);
    Serial.print(F("File size: "));
    Serial.print(binFile.fileSize()/512);
    Serial.println(F(" blocks"));
}

// -------------------------------- Send Function ------------------------------------
int send_readings(block_type *sd_block, int next_to_send) {
    int total_encoded = 0;
    do {
        int n_encoded = sp.encodeReadings(sd_block, next_to_send+total_encoded, BASE_MODE);
        total_encoded += n_encoded;
        int n_to_send = sp.getCurrentSize();
        // TODO: Test IRL, may be better to send in chunks different to n_to_send.
        int n_sent = softSerial.write(sp.getBuffer(), n_to_send);
        if (n_sent != sp.getCurrentSize()) {
            Serial.print(n_to_send);
            Serial.print(F(" bytes expected to send, but only sent: "));
            Serial.println(n_sent);
            // raise an error ?
        }
        sp.resetBuffer(); // Idea: "sp.resetPointer()" instead
                          // of memset'ing the buffer each time
    } while (total_encoded < sd_block->count);
    return 0;
}


// --------------------------------SD Setup ------------------------------------

void setup_sd() {
    // Wait for USB Serial
    if(sizeof(block_type) != 512) {
        error("block_type must have 512 bytes of length");
    }
    while (!Serial) {
        SysCall::yield();
    }
    Serial.print(F("Initializing SD ... "));
    // Initialize at the highest speed supported by the board that is
    // not over 50 MHz. Try a lower speed if SPI errors occur.
    if (!sd.begin(CHIP_SELECT, SD_SCK_MHZ(30))) {
        sd.initErrorHalt();
    }
    // Use of SD buffer as data block
    sd_block = (block_type *)sd.vol()->cacheClear();
    if (sd_block == 0) {
        error("cacheClear failed");
    }
    Serial.println(F("done"));
}

// ------------------------ Interruptions ---------------------------------------

void interrupt_sense() {
    alarm_flag |= SENSE_FLAG;
}

void interrupt_send() {
    alarm_flag |= SEND_FLAG;
}

void interrupt_both() {
    alarm_flag |= (SENSE_FLAG|SEND_FLAG);
}

// ------------------------ Sleep Routine ---------------------------------------

void sleep() {
    detachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN));
    sleep_enable();
    uint32_t dist;
    if(elapsed_time < SENDING_FREQ_SECS) {
        dist = SENDING_FREQ_SECS - elapsed_time;
        if(dist > SENSING_FREQ_SECS) {
            attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), interrupt_sense, FALLING);
            rtc.clearAlarm(1);
            alarm_time = rtc.now().unixtime() + SENSING_FREQ_SECS;
            rtc.setAlarm1(DateTime(alarm_time), DS3231_A1_Second);
            elapsed_time += SENSING_FREQ_SECS;
        }
        else if(dist < SENSING_FREQ_SECS) {
            attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), interrupt_send, FALLING);
            rtc.clearAlarm(1);
            alarm_time = rtc.now().unixtime() + dist;
            rtc.setAlarm1(DateTime(alarm_time), DS3231_A1_Second);
            elapsed_time += SENSING_FREQ_SECS;
        }
        else {
            attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), interrupt_both, FALLING);
            rtc.clearAlarm(1);
            alarm_time = rtc.now().unixtime() + dist;
            rtc.setAlarm1(DateTime(alarm_time), DS3231_A1_Second);
            elapsed_time = 0;
        }
    }
    else if(elapsed_time > SENDING_FREQ_SECS) {
        dist = elapsed_time - SENDING_FREQ_SECS;
        attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), interrupt_sense, FALLING);
        rtc.clearAlarm(1);
        alarm_time = rtc.now().unixtime() + dist;
        rtc.setAlarm1(DateTime(alarm_time), DS3231_A1_Second);
        elapsed_time = dist;
    }
    else {
        attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), interrupt_both, FALLING);
        rtc.clearAlarm(1);
        alarm_time = rtc.now().unixtime() + SENSING_FREQ_SECS;
        rtc.setAlarm1(DateTime(alarm_time), DS3231_A1_Second);
        elapsed_time = 0;
    }
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_cpu();
}

void logData(reading_type *rt, uint8_t pos) {
    rt->sensor = registered_sensors[pos];
    rt->ts = alarm_time;
    rt->val = sensors_functions[pos]();
    Serial.print(F("."));
}

//-------------------------- Main Program ------------------------------------------

void setup() {
    Serial.begin(9600);
    softSerial.begin(9600);  
    // ---------------------- RTC ----------------------
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, lets set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    // ----------------------- RTC Alarms ---------------
    rtc.disable32K();
    pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
    rtc.writeSqwPinMode(DS3231_OFF);
    rtc.disableAlarm(1);
    rtc.disableAlarm(2);
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    elapsed_time = 0;
    alarm_flag = 0;
    // ---------------------- DS18B20 ----------------------
    Serial.println(F("Starting DS18B20 sensor..."));
    sensors.begin();
    Serial.println(F("Done"));
    // ---------------------- ADC ----------------------
    Serial.println(F("Starting ADS1015..."));
    adc.begin();
    Serial.println(F("Done"));
    // ---------------------- SD ----------------------
    setup_sd();
    pending_blocks = 0;
    next_to_send = 0;
    // ---------------------- Binary File in SD ----------------------
    createBinFile();
}

void loop() {
    if(alarm_flag & SENSE_FLAG) {
        uint8_t available_slots = DATA_DIM - sd_block->count;
        if(available_slots < N_SENSORS) { // Block is full. Can't write all sensors inside the buffer
            Serial.println(F("Sd Block is full! Writting to file"));
            while(sd.card()->isBusy()); // Busy waiting
            if (!sd.card()->writeData((uint8_t *)sd_block)) { // Write buffer to SD
                error("write data failed");
            }
            Serial.println("Data written to file");
            if(++written_blocks == FILE_BLOCK_COUNT) { // If file is full, open another file
                Serial.println(F("Current file is full! Generating another file"));
                if (!sd.card()->writeStop()) {
                    error("writeStop failed");
                }
                renameBinFile();
                binFile.close();
                createBinFile();
                if(first_pending_file[0] == '#' && pending_blocks) { // Store the number of the first file generated 
                    Serial.println(F("Guardando en first_pending_file el archivo "));
                    Serial.println(binName);
                    first_pending_file[0] = binName[BASE_NAME_SIZE];
                    first_pending_file[1] = binName[BASE_NAME_SIZE+1];
                    first_pending_file[2] = binName[BASE_NAME_SIZE+2];
                    pending_blocks = 0;
                }
                next_to_send = 0;
            }
            else {
                if(next_to_send == sd_block->count) { // All data in buffer was sent
                    next_to_send = 0;
                }
                else if(first_pending_file[0] == '#') { // There is no pending file
                    ++pending_blocks;
                }
                sd_block->count = 0;
            }
        }
        for(uint8_t i=0; i<N_SENSORS; i++) { // Write data from all sensors
            logData(&sd_block->data[sd_block->count++], i);
        }
        alarm_flag &= ~SENSE_FLAG;
    }
    if(alarm_flag & SEND_FLAG) {
        if(first_pending_file[0] != '#') { // There are pending files to send
            Serial.println(F("There are pending files to send"));
            while(sd.card()->isBusy()); // Busy waiting
            if(sd_block->count) { // Write only if there is data in buffer
                if (!sd.card()->writeData((uint8_t *)sd_block)) {
                    error("write data failed");
                }
            }
            if (!sd.card()->writeStop()) {
                error("writeStop failed");
            }
            renameBinFile();
            binFile.close(); // Hasta aca no hay corrupción de la SD
            binName[BASE_NAME_SIZE] = first_pending_file[0];
            binName[BASE_NAME_SIZE+1] = first_pending_file[1];
            binName[BASE_NAME_SIZE+2] = first_pending_file[2];
            while(sd.exists(binName)) {
                Serial.println(binName);
                if (!binFile.open(binName, O_READ)) {
                      error("open failed");
                }
                while (binFile.read(sd_block, 512) == 512) {
                    if(sd_block->count == 0) {
                        Serial.println("No hay registros!");
                        break;
                    }
#if SEND_AS_PRINT
                    for(int i=0; i<sd_block->count; i++) {
                        Serial.print(sd_block->data[i].sensor);
                        Serial.print(" ");
                        Serial.print(sd_block->data[i].ts);
                        Serial.print(" ");
                        Serial.print(sd_block->data[i].val);
                        Serial.print("; ");
                    }
                    Serial.println();
#else // Real send (encoding)
                    send_readings(sd_block, next_to_send /* = 0 */);
#endif
                }
                binFile.close();
                if (binName[BASE_NAME_SIZE + 2] != '9') {
                    binName[BASE_NAME_SIZE + 2]++;
                }
                else if (binName[BASE_NAME_SIZE + 1] != '9') {
                    binName[BASE_NAME_SIZE + 2] = '0';
                    binName[BASE_NAME_SIZE + 1]++;
                }
                else if (binName[BASE_NAME_SIZE] != '9') {
                    binName[BASE_NAME_SIZE + 1] = '0';
                    binName[BASE_NAME_SIZE]++;
                }
            }
            createBinFile();
            first_pending_file[0] = '#';
            first_pending_file[1] = '#';
            first_pending_file[2] = '#'; 
        }
        else if(pending_blocks) { // There are blocks in the current binary file that hasn't been sent
            Serial.println(F("There are pending blocks to send"));
            while(sd.card()->isBusy()); // Busy waiting
            if(sd_block->count) { // Write only if there is data in buffer
                if (!sd.card()->writeData((uint8_t *)sd_block)) {
                    error("write data failed");
                }
            }
            if (!sd.card()->writeStop()) {
                error("writeStop failed");
            }
            renameBinFile();
            binFile.close();
            Serial.print(F("Abriendo el archivo"));
            Serial.println(binName);
            if (!binFile.open(binName, O_READ)) { // Open the previously created file
                error("open failed");
            }
            Serial.println("Abrí el archivo!");
            while (binFile.read(sd_block, 512) == 512) {
                if(sd_block->count == 0) {
                    Serial.println("No hay registros!");
                    break;
                }
#if SEND_AS_PRINT
                for(int i=next_to_send; i<sd_block->count; i++) {
                    Serial.print(sd_block->data[i].sensor);
                    Serial.print(" ");
                    Serial.print(sd_block->data[i].ts);
                    Serial.print(" ");
                    Serial.print(sd_block->data[i].val);
                    Serial.print("; ");
                }
                Serial.println();
                next_to_send = 0;
#else // Real send (encoding)
                send_readings(sd_block, next_to_send);
                next_to_send = 0;
#endif
            }
            Serial.println("Terminé el archivo");
            binFile.close();
            createBinFile();
            pending_blocks = 0;
        }
        else { // There is data in buffer to send
            Serial.println(F("There is data in buffer to send"));
#if SEND_AS_PRINT
            while(next_to_send < sd_block->count) {
                Serial.print(sd_block->data[next_to_send].sensor);
                Serial.print(" ");
                Serial.print(sd_block->data[next_to_send].ts);
                Serial.print(" ");
                Serial.print(sd_block->data[next_to_send].val);
                Serial.print("; ");
                next_to_send++;
            }
            Serial.println();
#else // Real send (encoding)
            send_readings(sd_block, next_to_send);
            next_to_send = 0;
#endif
        }
        alarm_flag &= ~SEND_FLAG;
    }
    sleep();
}
