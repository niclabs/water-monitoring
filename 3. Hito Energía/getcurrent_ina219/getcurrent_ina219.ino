#include <Wire.h>
#include <Adafruit_INA219.h>

#define DATALEN 100

Adafruit_INA219 ina219_0x40(0x40);
float data_buff[DATALEN] = { 0 };

void setup(void) {
	Serial.begin(115200);
	while (!Serial) {
		  // will pause Zero, Leonardo, etc until serial console opens
		  delay(1);
	}
	if (!ina219_0x40.begin()) {
	    Serial.println("Failed to find INA219 chip with adress 0x40");
	    while (1) { 
	        delay(10);
	    }
	}
	// To use a slightly lower 32V, 1A range (higher precision on amps):
	//ina219.setCalibration_32V_1A();
	// Or to use a lower 16V, 400mA range (higher precision on volts and amps):
	ina219_0x40.setCalibration_16V_400mA();
	}

void loop(void) {
  for(int i=0; i<DATALEN; ++i) {
      data_buff[i] = ina219_0x40.getCurrent_mA();
  }
  for(int i=0; i<DATALEN; ++i) {
      Serial.print(data_buff[i]);
      Serial.print(',');
      Serial.println(micros());
  }  
}
