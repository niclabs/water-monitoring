/*
 * Modified version of:
 * file DFRobot_PH.ino
 * @ https://github.com/DFRobot/DFRobot_PH
 *
 * Copyright   [DFRobot](https://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * version  V1.0
 * date  2018-04
 */

#include "DFRobot_PH.h"
#include <EEPROM.h>

#define PH_PIN A1
float voltage, phValue;
float temperature = NULL; // No se ocupa el valor de temperatura
DFRobot_PH ph;

void setup()
{
  Serial.begin(9600);
  ph.begin();
}

void loop()
{
    static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U)  // time interval: 1s
    {
      timepoint = millis();
      voltage = analogRead(PH_PIN)/1024.0*5000;  // read the voltage
      phValue = ph.readPH(voltage, temperature);  // convert voltage to pH
      Serial.print("^C  pH:");
      Serial.println(phValue, 2);
    }
    ph.calibration(voltage, temperature);  // calibration process by Serail CMD
}
