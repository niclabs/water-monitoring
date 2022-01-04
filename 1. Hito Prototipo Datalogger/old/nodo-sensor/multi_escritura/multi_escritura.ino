#include <Arduino.h>
#include "SdFat.h" // Bill Greiman
#include <avr/sleep.h>
#include "dataTypes.h"
#include <SoftwareSerial.h>
#include "RTClib.h"
#include <OneWire.h> 
#include <DallasTemperature.h>
SoftwareSerial softSerial(8,9);

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

// --------------------- File configuration variables --------------------------------
#define FILE_BASE_NAME "data"
#define TMP_FILE_NAME FILE_BASE_NAME "###.bin"
// Number of blocks of 512 bytes per file
const uint32_t FILE_BLOCK_COUNT = 2;
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
const uint8_t FILE_NAME_DIM  = BASE_NAME_SIZE + 8;
char binName[FILE_NAME_DIM] = FILE_BASE_NAME "000.bin";

//----------------------- SD configuration variables ---------------------------------
// SD chip select pin.
#define CHIP_SELECT 10
// SD Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
SdFat sd;
SdBaseFile binFile;
block_type *sd_block;
volatile uint16_t written_blocks;

//------------------------ Sensors configuration variables -----------------------  
// SENSING_FREQ_SECS must be less or equal to SENDING_FREQ_SECS
#define SENSING_FREQ_SECS 1
#define SENDING_FREQ_SECS 30
#define N_SENSORS 1
/* Sensors present on the data logger. If there are repeated sensors on the datalogger,
write them multiple times in the array.
For example, if there are 3 DS18B20 sensors, the array must contain the TEMP_DS18B20 value three times.
*/
uint8_t registered_sensors[N_SENSORS] = {
                                        TEMP_DS18B20
                                        };
// Typedef of function to read values from sensors
typedef float (*sensorValueFunction) (void);
/* User defined functions to read data from the registered_sensors array. Must return float and receive 
no parameters 
*/
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

void writeFile() {
    while(sd.card()->isBusy()); // Busy waiting
    if (!sd.card()->writeData((uint8_t *)sd_block)) { // Writting data
        error("write data failed");
    }
    Serial.println("Data written to file");
    if(++written_blocks == FILE_BLOCK_COUNT) { // Open another file
        Serial.println(F("Current file is full! Generating another file"));
        if (!sd.card()->writeStop()) {
            error("writeStop failed");
        }
        renameBinFile();
        binFile.close();
        createBinFile();
        written_blocks = 0;
        Serial.print("Abrí el archivo ");
        Serial.println(binName);
    }
    else {
        sd_block->count = 0;
    }
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
    // ---------------------- SD ----------------------
    setup_sd();
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
                written_blocks = 0;
            }
            else {
                sd_block->count = 0;
            }
        }
        for(uint8_t i=0; i<N_SENSORS; i++) { // Write data from all sensors
            logData(&sd_block->data[sd_block->count++], i);
        }
        alarm_flag &= ~SENSE_FLAG;
    }
    if(alarm_flag & SEND_FLAG) {
        Serial.println(F("Mandamos"));
        alarm_flag &= ~SEND_FLAG;
        delay(100);
    }
    sleep();
}
