/*
 * TEST DE PCB MCI
 * Pablo Martín, Matías Macaya
 * Programa que consulta al ADS1015 por sus valores cada x segundos.
 */

#include <Arduino.h>
#include "SdFat.h" // Bill Greiman
#include <avr/sleep.h>
#include <SoftwareSerial.h>
#include "RTClib.h"
#include <DS18B20.h>
#include <Adafruit_ADS1X15.h>

// ---------------------- ADC --------------------------
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

// ---------------------- Temp -------------------------
DS18B20 ds(9);

// ---------------------- RTC --------------------------
#define CLOCK_INTERRUPT_PIN 2
#define SENSE_FLAG 0x01
#define SEND_FLAG 0x02
#define SENSING_FREQ_SECS 10
RTC_DS3231 rtc;
uint32_t alarm_time;
volatile uint32_t elapsed_time;
volatile uint8_t alarm_flag;

int count = 0;

// ---------------------- Fx's -------------------------
void printTitle() {
  Serial.println("\n");
  Serial.println("************************************");
  Serial.println("NICLABS");
  Serial.println("Programa Revisión para PCB");
  Serial.println("************************************");
  Serial.println("Rodrigo Muñoz, Pablo Martín, Matías Macaya");
  Serial.println("************************************");
  Serial.println("Fx:");
  Serial.println("  - Consulta al ADS1015 y anota los resultados");
  Serial.println("  - Interrupción RTC al pin2 y sleep");
  Serial.println("************************************");
  Serial.print("Gain ADC: "); Serial.println(ads.getGain());
  Serial.println("");
}

void waking_up() {
  // Trigered with rtc wake up.
}

void sleep() {
  /**
  Set the Arduino Nano to sleep in Deep Sleep Mode (SLEEP_MODE_PWR_DOWN), after
  setting up the next RTC alarm.
  **/
  
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

//SETUP----------------------------------------------------------------------
void setup() {
  //Serial
  Serial.begin(9600);
  pinMode(3, OUTPUT); digitalWrite(3, HIGH); //Sensores
  pinMode(4, OUTPUT); digitalWrite(4, HIGH); //Serial
  
  // ADS1015
  ads.begin();
  ads.setGain(GAIN_ONE);
  printTitle();

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

  delay(5000);
}
  
//LOOP------------------------------------------------------------------------
void loop() {
  digitalWrite(3, HIGH); //Sensores
  digitalWrite(4, HIGH); //Serial
  
  Serial.println("pines high");
  delay(50);
  ads.begin();
  delay(50);
  
  int16_t adc0, adc1, adc2, adc3;
  float volts0, volts1, volts2, volts3;
  //Serial.println("--------------------");
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);
  
  // Write sensor data to CSV record.
  Serial.print(count);
  Serial.print(";\tA0_V = "); Serial.print(volts0);  // Presión
  Serial.print(";\tA1_V = "); Serial.print(volts1);  // Turbidez
  Serial.print(";\tA2_V = ");  Serial.print(volts2); // pH
  Serial.print(";\tA3_V = "); Serial.print(volts3);  // TDS
  while (ds.selectNext()) {
    Serial.print(";\tTemp C= "); Serial.println(ds.getTempC());
  }
  delay(10);
  
  count++;
  
  Serial.println("pines low");
  delay(1000);
  digitalWrite(3, LOW); //Sensores
  digitalWrite(4, LOW); //Serial
  sleep();
}
