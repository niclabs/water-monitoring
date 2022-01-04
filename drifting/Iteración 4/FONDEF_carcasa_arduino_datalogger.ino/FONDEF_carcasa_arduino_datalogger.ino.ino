// TABLA DE SENSORES ------------------------------------------------------
// id   nombre  piso  techo   unidad  unidad_ rango   param   error   tipo error
// 201  pH Pro  0   14  pH  pH  pH  0.1   +-
// 202  pH Blanco   0   14  pH  pH  pH  0.1   +-
// 203  pH Rojo   0   14  pH  pH  pH  0.1   +-
// 204  TDS Blanco  0   2000  uS/cm   uS/cm   CE  10  %
// 205  TDS Rojo  0   2000  uS/cm   uS/cm   CE  10  %
// 206  Presión agua  0   1200000   Pascal  Pascal  P   3   %
// 207  Temperatura atm   -40   85  ºC  ºC  T   1   +-
// 208  Presión atm   30000   110000  Pascal  Pascal  P   100   +-
// 209  Humedad atm   0   100   %   %   H   3   %
// 210  Temperatura agua 1  -10   85  ºC  ºC  T   0.5   +-
// 211  Temperatura agua 2  -10   85  ºC  ºC  T   0.5   +-

#include <Wire.h>
#include <Arduino.h>
#include "SdFat.h" // Bill Greiman
#include <avr/sleep.h>
#include <SoftwareSerial.h>
#include "RTClib.h"
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
#include "SdFat.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DS18B20.h>


#define VERBOSE_PRINT false
float volts0, volts1, volts2, volts3;
float t_atm, p_atm, h_atm;
float temp1, temp2;

bool bme_connected = true;

// ---------------------- DS18B20 ----------------------
DS18B20 ds(4);


// ---------------------- BME280 -----------------------
Adafruit_BME280 bme; // I2C

// ---------------------- ADC --------------------------
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

// ---------------------- RTC --------------------------
#define CLOCK_INTERRUPT_PIN 2
#define SENSE_FLAG 0x01
#define SEND_FLAG 0x02
#define SENSING_FREQ_SECS 20
RTC_DS3231 rtc;
uint32_t alarm_time;
volatile uint32_t elapsed_time;
volatile uint8_t alarm_flag;

// ----------------------- SD --------------------------
const uint8_t chipSelect = 10;  // SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
#define FILE_BASE_NAME "Data"   // Log file base name.  Must be six characters or less.
SdFat sd;                       // File system object.
SdFile file;                    // Log file.

#define error(msg) sd.errorHalt(F(msg))

void writeHeader() {
  file.print(F("unixtime"));
  file.print(F(",timestamp"));
  file.print(F(",A2 - Presión"));
  file.print(F(",A3 - NaN"));
  file.print(F(",T atm"));
  file.print(F(",P atm"));
  file.print(F(",H atm"));
  file.println();
}

void waking_up() {
  // Trigered with rtc wake up.
}

void sleep() {
  detachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN)); //De dónde recibirá la interrupción para despertar
  sleep_enable();
  uint32_t dist;
  
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), waking_up, FALLING);
  rtc.clearAlarm(1);
  alarm_time = rtc.now().unixtime() + SENSING_FREQ_SECS;
  rtc.setAlarm1(DateTime(alarm_time), DS3231_A1_Second);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
}

void(* resetFunc) (void) = 0;

// ---------------------- SET-UP --------------------------------------------
// --------------------------------------------------------------------------
void setup() {
  //digitalWrite(4, HIGH);
  //delay(200);
  //pinMode(4, OUTPUT);

  Serial.begin(9600);
  
  

  // -------------------- SD CARD ---------------------
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";
  
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    //sd.initErrorHalt();
    Serial.println("error init sd");
    delay(200);
    resetFunc();
    //digitalWrite(4, LOW);
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
    Serial.println("error file.open");
    delay(200);
    resetFunc();
    //error("file.open");
  }
  // Read any Serial data.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  if (VERBOSE_PRINT) {
    Serial.print(F("Logging to: "));
    Serial.println(fileName);
    Serial.println(F("Type any character to stop"));
  }

  // Write data header.
  writeHeader();

  // ---------------------- ADC ----------------------
  ads.begin();
  ads.setGain(GAIN_ONE);

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

  // ----------------------BME280 ---------------------
  unsigned status;
  status = bme.begin(0x76);
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      //while (1) delay(10);
      bme_connected = false;
  }

  delay(10000);
}
  
void loop() {
  //volts0 = ads.computeVolts(ads.readADC_SingleEnded(0));
  //volts1 = ads.computeVolts(ads.readADC_SingleEnded(1));
  //volts2 = ads.computeVolts(ads.readADC_SingleEnded(2));
  //volts3 = ads.computeVolts(ads.readADC_SingleEnded(3));

  temp1 = -1.0f;
  temp2 = -1.0f;
  t_atm = -1.0f;
  p_atm = -1.0f;
  h_atm = -1.0f;

  volts0 = ads.readADC_SingleEnded(0);
  volts1 = ads.readADC_SingleEnded(1);
  volts2 = ads.readADC_SingleEnded(2);
  volts3 = ads.readADC_SingleEnded(3);
  if (bme_connected) {
    t_atm = bme.readTemperature();
    p_atm = bme.readPressure() / 100.0F;
    h_atm = bme.readHumidity();
  }

  if (ds.selectNext()) temp1 = ds.getTempC();
  if (ds.selectNext()) temp2 = ds.getTempC();
  while (ds.selectNext()) {}

  DateTime now = rtc.now();
  file.print(now.unixtime(), DEC);
  file.print(',');
  file.print(now.year(), DEC);  file.print('/');
  file.print(now.month(), DEC); file.print('/');
  file.print(now.day(), DEC);   file.print(" ");
  file.print(now.hour(), DEC);  file.print(':');
  file.print(now.minute(), DEC);file.print(':');
  file.print(now.second(), DEC);
  file.write(',');file.print(volts2);
  file.write(',');file.print(volts3);
  file.write(',');file.print(t_atm);
  file.write(',');file.print(p_atm);
  file.write(',');file.print(h_atm);
  file.println();
  
  long unixTime = now.unixtime();
  Serial.print("{");
  Serial.print("\"app_id\": \"app123\",");
  Serial.print("\"dev_id\": \"rasp01\",");
  Serial.print("\"payload_raw\": \"[");
  Serial.print("{'i': 202, 'v':" + String(volts2) + ", 't': "); Serial.print(unixTime); Serial.print("000000000},");
  Serial.print("{'i': 203, 'v':" + String(volts3) + ", 't': "); Serial.print(unixTime); Serial.print("000000000},");
  Serial.print("{'i': 207, 'v':" + String(t_atm) + ", 't': "); Serial.print(unixTime); Serial.print("000000000},");
  Serial.print("{'i': 208, 'v':" + String(p_atm) + ", 't': "); Serial.print(unixTime); Serial.print("000000000},");
  Serial.print("{'i': 209, 'v':" + String(h_atm) + ", 't': "); Serial.print(unixTime); Serial.print("000000000}");
  Serial.print("]\"");
  Serial.print("}");
  Serial.print("\n");
  delay(50);
  
  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    error("write error");
  }
  
  delay(10);
  sleep();
}
