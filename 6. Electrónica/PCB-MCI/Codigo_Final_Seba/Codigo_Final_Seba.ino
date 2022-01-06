// Librerías Nativas (instaladas con Arduino)
#include <Arduino.h>
#include <avr/sleep.h>
//#include <SoftwareSerial.h>

// Librerías externas (respaldadas en repositorio)
#include "SdFat.h" // Bill Greiman
#include "SensorPayload.h"
#include "RTClib.h"
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>

/* Using temperature sensor is optional.
 * If USE_DS18B20 is set to 0, a dummy temperature
 * will be given each call (check function ds18b20value)
 * */
#define USE_DS18B20 1

#if USE_DS18B20
#include <DallasTemperature.h>
#endif

// ----------------- Debug option ----------------------
#define DEBUG 1
#if DEBUG
#define debugPrint(x) Serial.print(x)
#define debugPrintln(x) Serial.println(x)
#else
#define debugPrint(x) {}
#define debugPrintln(x) {}
#endif
#define ENCODE_MODE BASE_MODE

// ----------------- Software Serial port --------------
//SoftwareSerial softSerial(8,9);

// ---------------------- DS18B20 ----------------------
// DS18B20 setup variables
#define ONE_WIRE_BUS 3
// Setup a oneWire instance to communicate with any OneWire devices  
OneWire oneWire(ONE_WIRE_BUS); 
#if USE_DS18B20
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
#endif

// ---------------------- ADC ----------------------
Adafruit_ADS1015 adc;     /* Use this for the 12-bit version */

// ------------------ SensorPayload --------------------
#define N_READINGS_ENCODE 7
SensorPayload sp(N_READINGS_ENCODE * 7);

// ---------------------- RTC --------------------------
#define CLOCK_INTERRUPT_PIN 2
#define SENSE_FLAG 0x01
#define SEND_FLAG 0x02
RTC_DS3231 rtc;
uint32_t alarm_time;
uint32_t elapsed_time;
volatile uint8_t alarm_flag;

//------------------- LoPy timer -----------------------
unsigned long startTime;

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
#define USE_SD 1
#if USE_SD
// SD chip select pin.
#define CHIP_SELECT 10
// SD Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
SdFat sd;
SdBaseFile binFile;
block_type *sd_block;
uint16_t written_blocks;
#endif

//------------------------ Sensors configuration variables -----------------------  
// SENSING_FREQ_SECS must be less or equal to SENDING_FREQ_SECS
#define SENSING_FREQ_SECS 3
#define SENDING_FREQ_SECS 30 //168
#define N_SENSORS 5
// Typedef of function to read values from sensors
typedef float (*sensorValueFunction) (void);

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
#if USE_DS18B20
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
#else
    static int call_number = 1;
    float temp = 0.0 + call_number;
    call_number++;
#endif
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
    /**
    Create a new work file named TMP_FILE_NAME. If a previous version of TMP_FILE_NAME exists, it
    is removed and a new one is generated. The file generated has a size of FILE_BLOCK_COUNT * 512
    bytes, all initialized at value 0.
    **/
    // max number of blocks to erase per erase call
    const uint32_t ERASE_SIZE = 262144L;
    uint32_t bgnBlock, endBlock;
    // Delete old tmp file.
    if (sd.exists(TMP_FILE_NAME)) {
        debugPrintln(F("Deleting tmp file " TMP_FILE_NAME));
        if (!sd.remove(TMP_FILE_NAME)) {
            error("Can't remove tmp file");
        }
    }
    // Create new file.
    debugPrintln(F("\nCreating new file"));
    binFile.close();
    if (!binFile.createContiguous(TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
        error("createContiguous failed");
    }
    // Get the address of the file on the SD.
    if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
        error("contiguousRange failed");
    }
    // Flash erase all data in the file.
    debugPrintln(F("Erasing all data"));
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
    /**
    Rename the current work file binFile with a new unique name based on binName and the existing
    files stored in the SD card. This functions requires the binFile to store an open file, otherwise
    the work file can't be renamed.
    **/
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
    debugPrint(F("File renamed: "));
    debugPrintln(binName);
    debugPrint(F("File size: "));
    debugPrint(binFile.fileSize()/512);
    debugPrintln(F(" blocks"));
}

// --------------------------------SD Setup ------------------------------------

void setup_sd() {
    /**
    Start the variables associated with the SD card.
    **/
    /*
    // Wait for USB Serial
    if(sizeof(block_type) != 512) {
        error("block_type must have 512 bytes of length");
    }
    while (!Serial) {
        SysCall::yield();
    }
    debugPrint(F("Initializing SD ... "));
    // Initialize at the highest speed supported by the board that is
    // not over 50 MHz. Try a lower speed if SPI errors occur.
    if (!sd.begin(CHIP_SELECT, SD_SCK_MHZ(16))) {
        sd.initErrorHalt();
    }
    */
    // Use of SD buffer as data block
    sd_block = (block_type *)sd.vol()->cacheClear();
    if (sd_block == 0) {
        error("cacheClear failed");
    }
    debugPrintln(F("done"));
}
// ------------------------ Interruptions ---------------------------------------

void interrupt_sense() {
    /**
    Interrupt that sets the alarm_flag to trigger a sensing event.
    **/
    alarm_flag |= SENSE_FLAG;    
}

void interrupt_send() {
    /**
    Interrupt that sets the alarm_flag to trigger a sending event.
    **/
    alarm_flag |= SEND_FLAG;
}

void interrupt_both() {
    /**
    Interrupt that sets the alarm_flag to trigger a sensing and a sending event.
    **/
    alarm_flag |= (SENSE_FLAG|SEND_FLAG);    
}

// ------------------------ Sleep Routine ---------------------------------------

void sleep() {
    /**
    Set the Arduino Nano to sleep in Deep Sleep Mode (SLEEP_MODE_PWR_DOWN), after
    setting up the next RTC alarm.
    **/
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
    /**
    Log data from the sensor located in the position pos, inside the registered_sensors array.
    The data is stored in a reading_type struct, adressed by the pointer rt.
    **/
    rt->sensor = registered_sensors[pos];
    rt->ts = alarm_time;
    rt->val = sensors_functions[pos]();
    debugPrint(F("."));
}

void sendBuffer() {
    /**
    Send the current data stored in the SensorPayload sp.
    **/
    sp.addChecksum();
    int buf_size = sp.getCurrentSize();
    while(millis() - startTime < 1000); // Busy waiting used to wait one second for LoPy4 to awake.
    debugPrintln("Timer superado");
    int written = Serial.write(sp.getBuffer(), buf_size);
    sp.resetBuffer();
    debugPrint(F("Length of data sent: "));
    debugPrintln(written);
}

void sendCurrentBinFile() {
    /**
    Send the current open file stored in binFile, encoding the data stored inside it in the
    SensorPayload sp.
    **/
    if(!binFile.open(binName, O_READ)) {
        error("open failed");
    }
    while (binFile.read(sd_block, 512) == 512) {
        if(sd_block->count == 0) {
            break;
        }
        int encode_res;
        while(next_to_send < sd_block->count) {
            encode_res = sp.encodeReadings(sd_block, next_to_send, ENCODE_MODE);
            if(encode_res == 0) {
                sendBuffer();
            }
            next_to_send += encode_res;
        }
        next_to_send = 0;
    }
    binFile.close();
}

void wakeUpSender() {
    /**
    Send the value 0xFF to the LoPy4 to wake it up.
    **/
    debugPrintln("Sending 0xFF via serial");
    Serial.write(0xFF);
    startTime = millis();
}

//-------------------------- Main Program ------------------------------------------

void setup() {
    Serial.begin(19200);
    // ---------------------- RTC ----------------------
    if (!rtc.begin()) {
        debugPrintln(F("Couldn't find RTC"));
        while (1);
    }

    if (rtc.lostPower()) {
        debugPrintln(F("RTC lost power, lets set the time!"));
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
#if USE_DS18B20
    // ---------------------- DS18B20 ----------------------
    debugPrintln(F("Starting DS18B20 sensor..."));
    sensors.begin();
    debugPrintln(F("Done"));
#endif
    // ---------------------- ADC ----------------------
    debugPrintln(F("Starting ADS1015..."));
    if (!adc.begin()) {
        debugPrintln("Failed to initialize ADS.");
        while (1);
    }
    debugPrintln(F("Done"));
    // ---------------------- SD ----------------------
    setup_sd();
    pending_blocks = 0;
    next_to_send = 0;
    // ---------------------- Binary File in SD ----------------------
    //createBinFile();
}

void loop() {
    if(alarm_flag & SENSE_FLAG) {
        uint8_t available_slots = DATA_DIM - sd_block->count;
        if(available_slots < N_SENSORS) { // Block is full. Can't write all sensors inside the buffer
            debugPrintln(F("Sd Block is full! Writting to file"));
            //while(sd.card()->isBusy()); // Busy waiting
            //if (!sd.card()->writeData((uint8_t *)sd_block)) { // Write buffer to SD
            //    error("write data failed");
            //}
            debugPrintln("Data written to file");
            if(false) {//++written_blocks == FILE_BLOCK_COUNT) { // If file is full, open another file
                debugPrintln(F("Current file is full! Generating another file"));
                if (!sd.card()->writeStop()) {
                    error("writeStop failed");
                }
                renameBinFile();
                binFile.close();
                createBinFile();
                if(first_pending_file[0] == '#' && pending_blocks) { // Store the number of the first file generated 
                    first_pending_file[0] = binName[BASE_NAME_SIZE];
                    first_pending_file[1] = binName[BASE_NAME_SIZE+1];
                    first_pending_file[2] = binName[BASE_NAME_SIZE+2];
                    pending_blocks = 0;
                }
                next_to_send = 0;
            }
            else {
                if(true) {//next_to_send == sd_block->count) { // All data in buffer was sent
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
        wakeUpSender();
        if(first_pending_file[0] != '#') { // There are pending files to send
            debugPrintln(F("There are pending files to send"));
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
            binName[BASE_NAME_SIZE] = first_pending_file[0];
            binName[BASE_NAME_SIZE+1] = first_pending_file[1];
            binName[BASE_NAME_SIZE+2] = first_pending_file[2];
            while(sd.exists(binName)) {
                debugPrintln(binName);
                sendCurrentBinFile();
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
            if(sp.getCurrentSize()) {
                sendBuffer(); // There is remaining data not sent
            }
            createBinFile();
            first_pending_file[0] = '#';
            first_pending_file[1] = '#';
            first_pending_file[2] = '#';    
        }
        else if(pending_blocks) { // There are blocks in the current binary file that hasn't been sent
            debugPrintln(F("There are pending blocks to send"));
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
            debugPrintln(binName);
            sendCurrentBinFile();
            if(sp.getCurrentSize()) {
              sendBuffer(); // There is remaining data not sent
            }
            createBinFile();
            pending_blocks = 0;
        }
        else { // There is data in buffer to send
            debugPrintln(F("There is data in buffer to send"));
            int encode_res;
            while(next_to_send < sd_block->count) {
                encode_res = sp.encodeReadings(sd_block, next_to_send, ENCODE_MODE);
                sendBuffer();
                next_to_send += encode_res;
            }
        }
        alarm_flag &= ~SEND_FLAG;
    }
    sleep();
}
