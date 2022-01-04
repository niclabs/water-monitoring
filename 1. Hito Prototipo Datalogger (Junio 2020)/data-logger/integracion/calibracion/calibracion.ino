#include <Arduino.h>
//#include "SdFat.h" // Bill Greiman
//#include <avr/sleep.h>
//#include "SensorPayload.h"
//#include <SoftwareSerial.h>
//#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1015.h> // ADC

// ---------------------- DS18B20 ----------------------
// DS18B20 setup variables
#define ONE_WIRE_BUS 5
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temp_ds18b20(&oneWire);

// ---------------------- ADC ----------------------
/* The 4 channels of the ADC are allocated as follows:
    - Channel 0: TDS (EC) Sensor
    - Channel 1: pH Sensor
    - Channel 2: Turbidity Sensor
    - Channel 3: Pressure Sensor */
typedef enum {
    TDS_CHANNEL = 0,
    PH_CHANNEL,
    TURB_CHANNEL,
    PRES_CHANNEL
} adc_channel_t;

Adafruit_ADS1015 adc;     /* Use this for the 12-bit version */

#define ANALOG_TO_MV(a) ((a)/1024.0)*5000

float adc_voltage_reading(adc_channel_t channel) {
    int16_t reading = adc.readADC_SingleEnded(channel);
    return ANALOG_TO_MV(reading);
}

// Menus
String input_str;

// Sensors Configuration
/* The sensors which store their calibration parameters on the EEPROM are
    - TDS (EC) Sensor
    - pH Sensor */
#define ADDR_OFFSET 50 // <- Must be the same address the sensor code will look up.
float adc_reading;

/// ---------------------- Temp ----------------------
float getTemperature() {
    temp_ds18b20.requestTemperatures();
    return temp_ds18b20.getTempCByIndex(0);
}

/// ---------------------- EC ----------------------
#include <GravityTDS.h> // Use NicLabs version! https://github.com/niclabs/GravityTDS
GravityTDS tdsGravity(ADDR_OFFSET+1);

bool ec_done_1413 = false;
float temperature = 25;
void calibrate_tds() {
    while (1) {
        Serial.println(F("\nMenu de calibracion de C. E.:"));
        Serial.print(F("[1] Calibrar 1413 uS/cm ["));
        Serial.println(ec_done_1413 ? F("listo]") : F("NO LISTO]"));
        Serial.println(F("[9] Vuelta al menu principal"));
        while (!Serial.available());
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '1':
                adc_reading = adc_voltage_reading(TDS_CHANNEL); // mV
                adc_reading *= 1000; // reading in Volts
                temperature = getTemperature();
                tdsGravity.update(adc_reading, temperature);
                ec_done_1413 = tdsGravity.calibrate1413();
                if (!ec_done_1413) {
                  Serial.println(F("Fallo (¿La sonda no estaba en solucion de 1413 uS/cm?)"));
                }
                continue;
            case '9':
                return;
            default:
                Serial.println(F("[Opcion invalida]"));
                continue;
        }
    }
}

/// ---------------------- PH ----------------------
#include <DFRobot_PH.h> // Use NicLabs version! https://github.com/niclabs/DFRobot_PH/
DFRobot_PH phGravity(ADDR_OFFSET+2);

bool ph_done_acidic = false;
bool ph_done_neutral = false;
void calibrate_ph() {
    while (1) {
        Serial.println(F("\nMenu de calibracion de pH:"));
        Serial.print(F("[1] Calibrar Acido (pH 4) ["));
        Serial.println(ph_done_acidic ? F("listo]") : F("NO LISTO]"));
        Serial.print(F("[2] Calibrar Neutro (pH 7) ["));
        Serial.println(ph_done_neutral ? F("listo]") : F("NO LISTO]"));
        Serial.println(F("[9] Vuelta al menu principal"));
        while (!Serial.available());
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '1':
                adc_reading = adc_voltage_reading(PH_CHANNEL);
                phGravity.setValuesToCalibrate(adc_reading, NULL);
                ph_done_acidic = phGravity.calibrateAcidic();
                if (!ph_done_acidic) {
                  Serial.println(F("Fallo (¿La sonda no estaba en solucion de pH 4?)"));
                }
                continue;
            case '2':
                adc_reading = adc_voltage_reading(PH_CHANNEL);
                phGravity.setValuesToCalibrate(adc_reading, NULL);
                ph_done_neutral = phGravity.calibrateNeutral();
                if (!ph_done_neutral) {
                  Serial.println(F("Fallo (¿La sonda no estaba en solucion de pH 7?)"));
                }
                continue;
            case '9':
                return;
            default:
                Serial.println(F("[Opcion invalida]"));
                continue;
        }

    }
}

/// ---------------------- Turbidity ----------------------
/*
 * ¯\_(ツ)_/¯
 * */

/// ---------------------- Pressure ----------------------
// P=(Vout/5-0.1)*4/3

//-------------------------- Main Program ------------------------------------------

void setup() {
    Serial.begin(9600);
    // ---------------------- DS18B20 ----------------------
    Serial.println(F("Iniciando sensor DS18B20..."));
    temp_ds18b20.begin();
    Serial.println(F("DS18B20 listo."));
    // ---------------------- ADC ----------------------
    Serial.println(F("Iniciando ADS1015..."));
    adc.begin();
    Serial.println(F("ADC listo."));
    // ---------------------- EC ----------------------
    Serial.println(F("Iniciando sensor de C. E. (TDS)..."));
    tdsGravity.begin();
    Serial.println(F("Sensor TDS listo"));
    // ---------------------- PH ----------------------
    Serial.println(F("Iniciando sensor pH..."));
    phGravity.begin();
    Serial.println(F("Sensor pH listo"));
}

void loop() {
    while (1) {
        Serial.println(F("\nMenu principal:"));
        Serial.println(F("[1] Calibrar sensor de pH"));
        Serial.println(F("[2] Calibrar sensor de cond. eléctrica (TDS)"));
        Serial.println();
        while (!Serial.available());
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '1':
                calibrate_ph();
                continue;
            case '2':
                calibrate_tds();
                continue;
            default:
                Serial.println(F("[Opcion invalida]"));
                continue;
        }

    }
}
