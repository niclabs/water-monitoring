//##############################################
//#################### OLED ####################
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>

// Nano
#define DATA_PIN A4
#define CLOCK_PIN A5

// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#ssd1306-128x64_noname-1
U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, CLOCK_PIN, DATA_PIN);

const int x_text_left = 10;
const int y_title = 5+5;
const int y_box1 = 15;
const int y_box2 = 40;
const int text_offset = 5+5;

// Called by showMeasurement. Just decoration.
void _drawLayout() {
  // title
  //u8g2.drawStr(x_text_left, y_title, "Last measurement:"); // +24 bytes progmem
  u8g2.setCursor(x_text_left, y_title);
  u8g2.print(F("Last measurement:"));

  // 1st box     (x,y,dx,dy,round)
  u8g2.drawRFrame(5, y_box1, 70, 15, 5);
  
  // 2nd box
  u8g2.drawRFrame(5, y_box2, 120, 15, 5);
}


void showMeasurement(uint16_t value /*, int precission*/, char unit[], char tmstamp[]) {
  u8g2.firstPage();
  do {
    _drawLayout();
 
    // value 1st box 
    u8g2.setCursor(x_text_left, y_box1 + text_offset);
    u8g2.print(value /*, precission */);
  
    // outside 1st box (unit)
    u8g2.setCursor(80, y_box1 + text_offset);
    u8g2.print(unit);
  
    // value 2nd box (timestamp)
    u8g2.setCursor(x_text_left, y_box2 + text_offset);
    u8g2.print(tmstamp);

  } while ( u8g2.nextPage() );
}

//#################################################
//#################### SD_CARD ####################
#include "SdFat.h" // Bill Greiman

// File system object.
SdFat sd;

// Sensors
const uint8_t DATA_COUNT = 2;
static uint16_t data_buffer[DATA_COUNT] = {0};
#define ANALOG_PIN A0

// Log file.
SdFile file;

// Variables for button
long prev = 0;
byte buttonState = 0;
#define LONG_PRESS_DURATION (2000)
#define SHORT_PRESS (1)
#define LONG_PRESS (2)

//----------------------- SD User configuration ---------------------------------

// SD chip select pin.
const byte chipSelect = 10;
// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"
// Pin where button is connected.
#define BUTTON_PIN (6)


// Write data header.
void writeHeader() {
  String header_elem[DATA_COUNT] = {"timestamp","analog1"};
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
  // Lectura del primer valor del arreglo
  file.print(data_buffer[0]);
  // Write ADC data to CSV record.
  for (uint8_t i = 1; i < DATA_COUNT; i++)
  {
    file.write(',');
    file.print(data_buffer[i]);
  }
  file.println();
}

//------------------------------------------------------------------------------

// SD Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))


//################################################
//####################  Reloj ####################
#include "RTClib.h"
RTC_DS3231 rtc;

void printTitle() {
  DateTime now = rtc.now();
  Serial.print("\n\n[");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print("] Inicio del programa");
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

  //##############################################
  //#################### OLED ####################
  Serial.println(F("Setting screen... "));
  setup_screen();
  Serial.println(F("Done."));
  Serial.println(F("-----------------------\n"));

  //#################################################
  //#################### SD_CARD ####################
  Serial.print(F("Setting SD... "));
  //setup_sd();
  Serial.println(F("Done."));

  //################################################
  //#################### BUTTON ####################
  attachInterrupt(0, buttonPressed, RISING); // int 0 = pin 2
}

void setup_screen() {
  u8g2.begin();
  // https://github.com/olikraus/u8g2/wiki/fntlistmono
  u8g2.setFont(/*u8g2_font_ncenB14_tr*/ u8g2_font_6x10_mf);
}

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

uint8_t count = 0; // rollover in ~30 seconds

void buttonPressed() {
  buttonState = 1;
}

void loop() {
  //showMeasurement(3.141592 + count, 2, "\xF8 C", "2020-02-30 10:34:56");
  //count++;
  //delay(500);
  if (buttonState == 1) {
    if (digitalRead(BUTTON_PIN) == HIGH) {
      delay(50);
      if (digitalRead(BUTTON_PIN) == HIGH) {
        prev = millis();
        buttonState = LONG_PRESS;
        while ((millis() - prev) <= LONG_PRESS_DURATION) {
          if (digitalRead(BUTTON_PIN) == LOW) {
            buttonState = SHORT_PRESS;
            break;
          }
        }
      } 
    }

    if (buttonState == LONG_PRESS) {
      file.close(); // Close file and stop.
      Serial.println(F("Logging done."));
      SysCall::halt();
      
    } else if (buttonState == SHORT_PRESS) {
      Serial.println(F("Reading from Analog"));
      int turbidity = analogRead(ANALOG_PIN);
      showMeasurement(turbidity, "u", "12345");
      logData(0, 12345); // dummy time
      logData(1, turbidity);
      Serial.print(F("Writing to SD... "));
      writeData();
  
      // Force data to SD and update the directory entry to avoid data loss.
      if (!file.sync() || file.getWriteError()) {
          error("write error");
      }
      Serial.println(F("done"));
      delay(1000);
    }

    buttonState = 0;
  }
}
