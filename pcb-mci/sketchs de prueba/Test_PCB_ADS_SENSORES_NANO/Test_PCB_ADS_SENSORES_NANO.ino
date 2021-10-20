/*
 * TEST DE PCB MCI - PROGRAMA PARA Arduino Nano
 * Pablo Martín, Matías Macaya
 * Programa que consulta al ADS1015 por sus valores cada x segundos.
 */

//Librerías:
  //ADS1015
  #include <Wire.h>
  #include <Adafruit_ADS1X15.h>
  #include <DS18B20.h>
  #include <SPI.h>
  #include "SdFat.h"

//Creación de instancias ADS, DS18B20
// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
DS18B20 ds(9);


//SETUP----------------------------------------------------------------------
void setup() {
  //Serial
    Serial.begin(9600);
  // ADS1015
    ads.begin();
    ads.setGain(GAIN_ONE);   
    float gain = ads.getGain();
    Serial.println("************************************");
    Serial.println("NICLABS");
    Serial.println("Programa Revisión de sensores para Arduino Nano");
    Serial.println("************************************");
    Serial.println("Rodrigo Muñoz, Pablo Martín, Matías Macaya");
    Serial.println("************************************");
    Serial.println("Consulta al ADS1015 y anota los resultados");

    Serial.print("Gain ADC: "); Serial.println(gain);
  }
//------------------------------------------------------------------------------
void loop() {
  int16_t adc0, adc1, adc2, adc3;
  float volts0, volts1, volts2, volts3;
  Serial.println("--------------------");
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);
  
  // Write sensor data to CSV record.
  Serial.print("A0 TDS V = "); Serial.println(volts0); 
  Serial.print("A1 PH V = "); Serial.println(volts1); 
  Serial.print("A2 TURB V = ");  Serial.println(volts2); 
  Serial.print("A3 PRES V = "); Serial.println(volts3); 
  while (ds.selectNext()) {
    Serial.print("Temp C= "); Serial.println(ds.getTempC());
  }
  delay(2000);
}
