/*
 * Original code: dataLogger example from sdFat library for Arduino
 * 
 */
#include <SPI.h>
#include "SdFat.h"

//------------------------------------------------------------------------------
// File system object.
SdFat sd;

// Log file.
SdFile file;

// Variables for button
long prev = 0;
byte buttonState = 0;
#define LONG_PRESS_DURATION (2000)
#define SHORT_PRESS (1)
#define LONG_PRESS (2)

//--------------------User configuration------------------------------------------

// SD chip select pin.
const byte chipSelect = 10;
// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"
// Pin where button is connected.
#define BUTTON_PIN (6)


const uint8_t DATA_COUNT = 4;

// Write data header.
void writeHeader() {
  String header_elem[DATA_COUNT] = {"timestamp","analog1","analog2","analog3"};
  file.print(header_elem[0]);
  for (uint8_t i = 1; i < DATA_COUNT; i++) {
    file.write(',');
    file.print(header_elem[i]);
  }
  file.println();
}

// Log a data record.
void logData() {
  uint16_t data[DATA_COUNT];
  // Read the data from different sources
  for (uint8_t i = 0; i < DATA_COUNT; i++) {
    data[i] = analogRead(i);
  }
  // Lectura del primer valor del arreglo
  file.print(data[0]);
  // Write ADC data to CSV record.
  for (uint8_t i = 1; i < DATA_COUNT; i++) {
    file.write(',');
    file.print(data[i]);
  }
  file.println();
}
//------------------------------------------------------------------------------
// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))

void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  Serial.begin(9600);
  
  // Wait for USB Serial 
  while (!Serial) {
    SysCall::yield();
  }
  delay(1000);
  pinMode(BUTTON_PIN, INPUT);
  Serial.print(F("Initializing SD ... "));
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }
  Serial.println(F("done"));
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
  
  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  Serial.println(F("Press button short to log data"));
  Serial.println(F("Mantain button pressed to stop logging"));
  // Write data header.
  writeHeader();
}

void loop() {
  buttonState = 0;
  if(digitalRead(BUTTON_PIN) == HIGH){
    delay(50);
    if(digitalRead(BUTTON_PIN) == HIGH) {
      prev = millis();
      buttonState = LONG_PRESS;
      while((millis()-prev)<=LONG_PRESS_DURATION){
        if((digitalRead(BUTTON_PIN)) == LOW){
          buttonState = SHORT_PRESS;
          break;
        }
      }
    } 
  }
 

  if(buttonState == LONG_PRESS){
    // Close file and stop.
    file.close();
    Serial.println(F("Logging done."));
    SysCall::halt();
  }else if(buttonState == SHORT_PRESS){
    Serial.print(F("Writting to SD... "));
    logData();
    // Force data to SD and update the directory entry to avoid data loss.
    if (!file.sync() || file.getWriteError()) {
    error("write error");
    }
    Serial.println(F("done"));
    delay(1000);
  }
}
