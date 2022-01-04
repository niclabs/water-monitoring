#include <SoftwareSerial.h>
#include <SensorPayload.h>
#include <time.h>

/*
 * typedef struct data_block { // Greiman
    uint16_t count; // Number of readings on block
    reading_type data[DATA_DIM]; // Readings
    uint8_t fill[FILL_DIM]; // Bytes to fill 512 bytes
} block_type;
 */

block_type block; // Local, no estamos usando una SD
/*
 * typedef struct sensor_reading {
    float val;
    uint32_t ts;
    uint8_t sensor;
    } reading_type;
 */
#define N_READINGS 5
reading_type dummy_readings[N_READINGS] = {{2.17828, 0, TEMP_DS18B20},
                                           {3.14159, 0, TEMP_SHT20},
                                           {1.41421, 0, PRES_GRAVITY},
                                           {1.61803, 0, PRES_HK1100C},
                                           {0.69314, 0, PH_GRAVITY}};

SensorPayload sp(N_READINGS*7); // 7 bytes por Sensing Unit

// Comunicación serial
SoftwareSerial softSerial(8,9); // myRX, myTX

void setup() {
  Serial.begin(9600);
  softSerial.begin(19200);
  
  Serial.println(F("Creating timestamps"));
  for (int i=0; i<N_READINGS; i++) {
    dummy_readings[i].ts = millis();
    block.data[i] = dummy_readings[i];
    delay(10);
  }
  block.count = N_READINGS;

  Serial.println(F("Encoding readings"));

  softSerial.write((uint8_t) 0xa); // Enviar un byte para despertar a la LoPy

  int n_encoded = 0;
  while (n_encoded < N_READINGS) {
    n_encoded += sp.encodeReadings(&block, 0, BASE_MODE);
    uint8_t *sp_buffer = sp.getBuffer();
    Serial.print(F("Readings encoded so far: "));
    Serial.println(n_encoded, DEC);
    Serial.print(F("Buffer content: 0x"));
    for (int i=0; i<sp.getCurrentSize(); i++) {
      /* Lo printeado aquí son los bytes a enviar */
      // Comparar con lo que entrega
      // https://github.com/niclabs/water-monitoring/blob/master/energy/decoder/payloadDecoder.py
      // Para el ejemplo, me dio:
      // [(0, 0, 2.1), (1, 9, 3.1), (2, 19, 1.4), (3, 29, 1.6), (4, 39, 0.6)]
      // Funciona
      if (sp_buffer[i] < 0x10) {
        Serial.print('0');
      }
      Serial.print(sp_buffer[i], HEX);

      softSerial.write(sp_buffer[i]);
    }
    Serial.println();
  }

}

void loop() {}
