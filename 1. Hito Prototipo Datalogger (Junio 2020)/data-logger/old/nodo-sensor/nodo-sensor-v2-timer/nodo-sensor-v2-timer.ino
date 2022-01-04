//##############################################
//#################### OLED ####################
#include <Arduino.h>
#include <SPI.h>
#include <U8x8lib.h>

#define DATA_PIN A4
#define CLOCK_PIN A5

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

void showMeasurement(uint16_t value /*, int precission*/, char unit[], char tmstamp[]) {
  u8x8.drawString(0, 0, "Last measurement");
  u8x8.setCursor(1, 1);
  u8x8.print(value);
  u8x8.drawString(0, 3, "at");
  u8x8.drawString(1, 4, unit);
  u8x8.drawString(1, 5, tmstamp);
}


//#################################################
//#################### SD_CARD ####################
#include "SdFat.h" // Bill Greiman

// File system object.
SdFat sd;

// Sensors
const uint8_t DATA_COUNT = 3;
static uint16_t data_buffer[DATA_COUNT] = {0};
String header_elem[DATA_COUNT] = {"timestamp","analog1", "rtc_temp"};
#define ANALOG_PIN A0

SdFile file;  // Log file.

// Variables for button
byte buttonState = 0;

//----------------------- SD User configuration ---------------------------------

const byte chipSelect = 4;    // SD chip select pin.
#define FILE_BASE_NAME "Data" // Log file base name.  Must be six characters or less.
#define BUTTON_PIN (2)        // Pin where button is connected.

// Write data header.
void writeHeader() {
  //String header_elem[DATA_COUNT] = {"timestamp","analog1"};
  file.print(header_elem[0]);
  for (uint8_t i = 1; i < DATA_COUNT; i++) {
    file.write(',');
    file.print(header_elem[i]);
  }
  file.println();
}

// Log a data into buffer.
void logData(uint8_t index, int data) {
  data_buffer[index] = data;
}

// Write data buffer to SD
void writeData() {
  file.print(data_buffer[0]);                  // Lectura del primer valor del arreglo
  for (uint8_t i = 1; i < DATA_COUNT; i++) {   // Write ADC data to CSV record.
    file.write(',');
    file.print(data_buffer[i]);
  }
  file.println();
}

//------------------------------------------------------------------------------

#define error(msg) sd.errorHalt(F(msg))        // SD Error messages stored in flash.

void setup_sd() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  delay(1000);
  pinMode(BUTTON_PIN, INPUT);
  Serial.print(F("Initializing SD ... "));
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) { // Initialize at the highest speed supported by the board that is
    sd.initErrorHalt();                        // not over 50 MHz. Try a lower speed if SPI errors occur.
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
  writeHeader();  // Write data header.
}


//################################################
//####################  Reloj ####################
#include "RTClib.h"
RTC_DS3231 rtc;
DateTime now;

void printTitle() {
  DateTime now = rtc.now();
  Serial.print("\n\n[");
  Serial.print(now.year(), DEC);      Serial.print('/');
  Serial.print(now.month(), DEC);     Serial.print('/');
  Serial.print(now.day(), DEC);       Serial.print(' ');
  Serial.print(now.hour(), DEC);      Serial.print(':');
  Serial.print(now.minute(), DEC);    Serial.print(':');
  Serial.print(now.second(), DEC);    Serial.print("] Inicio del programa");
  Serial.println();
}


//#################################################
//####################  SET-UP ####################
void setup() {
  Serial.begin(9600);

  //################################################
  //####################  Reloj ####################
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  printTitle();
  now = rtc.now();

  //##############################################
  //#################### OLED ####################
  Serial.println(F("Setting screen... "));
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,0,"Inicio 01");
  
  Serial.println(F("Done."));
  Serial.println(F("-----------------------\n"));

  //#################################################
  //#################### SD_CARD ####################
  Serial.print(F("Setting SD... "));
  setup_sd();
  Serial.println(F("Done."));

  //################################################
  //################## INTERRUPT ###################
  noInterrupts();
  TCCR1A = 0;               // timer 1
  TCCR1B = 0;

  TCNT1 = 3036;             // preload timer 65536-16MHz/1024/0.25Hz(4s)
  TCCR1B |= (1 << CS12);    // 1024 prescaler 
  TCCR1B |= (1 << CS10);    // 1024 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

uint8_t count = 0;          // rollover in ~30 seconds

ISR(TIMER1_OVF_vect) {      // interrupt service routine that wraps a user defined function supplied by attachInterrupt
  TCNT1 = 3036;             // preload timer
  buttonState = 1;
}

void loop() {
  if (buttonState == 1) {
    Serial.println(F("Reading from Analog"));
    int turbidity = analogRead(ANALOG_PIN);

    Serial.println(F("Printing in oled"));
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.print(count++);

    Serial.print(F("Writing to SD... "));
    logData(0, now.unixtime()); // dummy time
    logData(1, turbidity);
    logData(2, rtc.getTemperature());
    writeData();

    // Force data to SD and update the directory entry to avoid data loss.
    if (!file.sync() || file.getWriteError()) {
        error("write error");
    }
    Serial.println(F("done"));
  }

  buttonState = 0;
}
