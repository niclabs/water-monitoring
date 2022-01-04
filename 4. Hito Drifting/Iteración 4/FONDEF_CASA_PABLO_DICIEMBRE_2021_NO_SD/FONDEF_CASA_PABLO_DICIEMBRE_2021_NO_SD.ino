// 
// Programa V1 incorpora 1 solo ADC ADS1015
// Programa V2 incorpora 6 sensores análogos en dos ADS1015
// ...
// ...
// 

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
float volts4, volts5;
float t_atm, p_atm, h_atm;
float temp1, temp2;

bool bme_connected = true;

// ---------------------- DS18B20 ----------------------
DS18B20 ds(4);


// ---------------------- BME280 -----------------------
Adafruit_BME280 bme; // I2C

// ---------------------- ADC --------------------------
Adafruit_ADS1015 ads1;     /* Use this for the 12-bit version */
Adafruit_ADS1015 ads2;     /* Use this for the 12-bit version */

// ---------------------- RTC --------------------------
#define CLOCK_INTERRUPT_PIN 2
#define SENSE_FLAG 0x01
#define SEND_FLAG 0x02
#define SENSING_FREQ_SECS 1
RTC_DS3231 rtc;
uint32_t alarm_time;
volatile uint32_t elapsed_time;
volatile uint8_t alarm_flag;

// ----------------------- SD --------------------------
#define CHIP_SELECT 10  // SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
#define FILE_BASE_NAME "Data"   // Log file base name.  Must be six characters or less.
SdFat sd;                       // File system object.
SdFile file;                    // Log file.

#define error(msg) sd.errorHalt(F(msg))

void writeHeader() {
  file.println(F("unixtime,timestamp,A0,A1,A2,A3,A4,A5,T atm,P atm,H atm,Temp1,Temp2"));
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

// ---------------------- SET-UP --------------------------------------------
// --------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  
  // ---------------------- ADC ----------------------
  ads1.begin(0x48);
  ads1.setGain(GAIN_ONE);
  ads2.begin(0x49);
  ads2.setGain(GAIN_ONE);

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
  unsigned status = bme.begin(0x76);
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      //while (1) delay(10);
      bme_connected = false;
  }

  delay(1000);
}
  
void loop() {
  temp1 = -1.0f;
  temp2 = -1.0f;
  t_atm = -1.0f;
  p_atm = -1.0f;
  h_atm = -1.0f;

  volts0 = ads1.readADC_SingleEnded(0);
  volts1 = ads1.readADC_SingleEnded(1);
  volts2 = ads1.readADC_SingleEnded(2);
  volts3 = ads1.readADC_SingleEnded(3);
  volts4 = ads2.readADC_SingleEnded(0);
  volts5 = ads2.readADC_SingleEnded(1);
  if (bme_connected) {
    t_atm = bme.readTemperature();
    p_atm = bme.readPressure() / 100.0F;
    h_atm = bme.readHumidity();
  }

  if (ds.selectNext()) temp1 = ds.getTempC();
  if (ds.selectNext()) temp2 = ds.getTempC();
  while (ds.selectNext()) {}

  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);  Serial.print('/');
  Serial.print(now.month(), DEC); Serial.print('/');
  Serial.print(now.day(), DEC);   Serial.print(" ");
  Serial.print(now.hour(), DEC);  Serial.print(':');
  Serial.print(now.minute(), DEC);Serial.print(':');
  Serial.print(now.second(), DEC);Serial.print(",\t");
  Serial.print(String(volts0)+ ",\t");
  Serial.print(String(volts1)+ ",\t");
  Serial.print(String(volts2)+ ",\t");
  Serial.print(String(volts3)+ ",\t");
  Serial.print(String(volts4)+ ",\t");
  Serial.print(String(volts5)+ ",\t");
  Serial.print(String(t_atm) + ",\t");
  Serial.print(String(p_atm) + ",\t");
  Serial.print(String(h_atm) + ",\t");
  Serial.print(String(temp1) + ",\t");
  Serial.print(String(temp2));
  Serial.print("\n");
  delay(1000);
  //sleep();
}
