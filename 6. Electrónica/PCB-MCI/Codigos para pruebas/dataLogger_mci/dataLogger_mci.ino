/*
 * SYSTEM: DEFINIR QUE FUNCIONALIDADES SE QUIEREN HABILITAR
 */
#define SDCARD_ON false
#define BATTERY_ON HIGH
#define SENSORS_ON HIGH
#define RS485_ON HIGH


/*
 * ADS1015
 */
#include <Wire.h>
#include <Adafruit_ADS1015.h>

// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

/*
 * DS18B20
 */
#include <DS18B20.h>

DS18B20 ds(9);

/*
 * RTC DS3231.
 */
#include "RTClib.h"
RTC_DS3231 rtc;

/*
 * Simple data logger.
 */
#include <SPI.h>
#include "SdFat.h"

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint16_t TIME_INTERVAL = 5;

const uint8_t chipSelect = 10;  // SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
#define FILE_BASE_NAME "Data"   // Log file base name.  Must be six characters or less.
SdFat sd;                       // File system object.
SdFile file;                    // Log file.
uint32_t logTime;               // Time in micros for next data record.
uint32_t nextMeasureTime;           // Next time to measure in the script
DateTime now;                   // timestamp for printing in file

//==============================================================================
// User functions.  Edit writeHeader() and logData() for your requirements.

const uint8_t ANALOG_COUNT = 4;

//------------------------------------------------------------------------------
// RTC - Write data header.

//------------------------------------------------------------------------------
// Write data header.
void writeHeader() {
  file.print(F("timestamp"));
  file.print(F(",prss []"));
  file.print(F(",turb []"));
  file.print(F(",pH []"));
  file.print(F(",CE []"));
  file.print(F(",temp []"));
  file.println();
}
//------------------------------------------------------------------------------
// Log a data record.
void logData() {
  
  // Write data to file.  Start with log timestamp.
  file.print(now.year(), DEC);    file.print('/');
  file.print(now.month(), DEC);   file.print('/');
  file.print(now.day(), DEC);     file.print(" ");
  file.print(now.hour(), DEC);    file.print(':');
  file.print(now.minute(), DEC);  file.print(':');
  file.print(now.second(), DEC);

  // Write sensor data to CSV record.
  file.write(',');file.print(ads.readADC_SingleEnded(0));
  file.write(',');file.print(ads.readADC_SingleEnded(1));
  file.write(',');file.print(ads.readADC_SingleEnded(2));
  file.write(',');file.print(ads.readADC_SingleEnded(3));
  while (ds.selectNext()) {
    file.write(',');file.print(ds.getTempC());
  }
  file.println();
}
//------------------------------------------------------------------------------
// Print data record on serial port.
void printData() {
  
  // Write data to file.  Start with log timestamp.
  Serial.print(now.year(), DEC);    Serial.print('/');
  Serial.print(now.month(), DEC);   Serial.print('/');
  Serial.print(now.day(), DEC);     Serial.print(" ");
  Serial.print(now.hour(), DEC);    Serial.print(':');
  Serial.print(now.minute(), DEC);  Serial.print(':');
  Serial.print(now.second(), DEC);

  // Write sensor data to CSV record.
  Serial.write(",\t");Serial.print(ads.readADC_SingleEnded(0));
  Serial.write(",\t");Serial.print(ads.readADC_SingleEnded(1));
  Serial.write(",\t");Serial.print(ads.readADC_SingleEnded(2));
  Serial.write(",\t");Serial.print(ads.readADC_SingleEnded(3));
  while (ds.selectNext()) {
    Serial.write(",\t");Serial.print(ds.getTempC());
  }
  Serial.println();
}

//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))

//------------------------------------------------------------------------------
void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  pinMode(2, OUTPUT); digitalWrite(2, BATTERY_ON);  // Habilitar batería
  pinMode(3, OUTPUT); digitalWrite(3, SENSORS_ON);  // Habilitar fuente para sensores
  pinMode(4, OUTPUT); digitalWrite(4, RS485_ON);  // Habilitar RS485
  
  Serial.begin(9600);
  
  // Wait for USB Serial 
  //while (!Serial) {
  //  SysCall::yield();
  //}
  delay(1000);


  // ADS1015
  ads.begin();

  // DS18B20
  // nada

  // RTC DS3231
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  Serial.println("-- DS3231 initialized");


  // SD Cards
  //Serial.println(F("Type any character to start"));
  //while (!Serial.available()) {
  //  SysCall::yield();
  //}
  
  if (SDCARD_ON) {
    // Initialize at the highest speed supported by the board that is
    // not over 50 MHz. Try a lower speed if SPI errors occur.
    if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
      sd.initErrorHalt();
    }
  
    // Find an unused file name.
    if (BASE_NAME_SIZE > 6) {
      error("FILE_BASE_NAME too long");
    }
    while (sd.exists(fileName)) {
      if (fileName[BASE_NAME_SIZE + 1] != '9') {
        fileName[BASE_NAME_SIZE + 1]++;
      } else if (fileName[BASE_NAME_SIZE] != '9') {
        fileName[BASE_NAME_SIZE + 1] = '0';
        fileName[BASE_NAME_SIZE]++;
      } else {
        error("Can't create file name");
      }
    }
    if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
      error("file.open");
    }
    // Read any Serial data.
    do {
      delay(10);
    } while (Serial.available() && Serial.read() >= 0);
  
    Serial.print(F("Logging to: "));
    Serial.println(fileName);
    Serial.println(F("Type any character to stop"));
  
    // Write data header.
    writeHeader();
  }

  now = rtc.now();
  nextMeasureTime = now.unixtime() + TIME_INTERVAL;
}
//------------------------------------------------------------------------------
void loop() {
  
  int32_t diff;
  do {
    now = rtc.now();
    diff = nextMeasureTime - now.unixtime();
    delay(10);
  } while (diff > 0);

  if (SDCARD_ON) {
    logData();
  }
  printData();

  nextMeasureTime += 5;

  if (SDCARD_ON) {
    // Force data to SD and update the directory entry to avoid data loss.
    if (!file.sync() || file.getWriteError()) {
      error("write error");
    }
  }

  if (Serial.available()) {
    // Close file and stop.
    if (SDCARD_ON) {
      file.close();
    }
    Serial.println(F("Done"));
    SysCall::halt();
  }
}
