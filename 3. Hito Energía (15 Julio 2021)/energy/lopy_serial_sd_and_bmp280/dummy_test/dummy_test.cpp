//#include <SoftwareSerial.h>
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
//SoftwareSerial softSerial(8,9); // myRX, myTX

// <Portado a C>
#include <unistd.h> // time
#include <cstdio> // printf
#define F(arg) arg
#define CHAR 0
#define DEC 1
#define HEX 2
class Cerial {
public:
    Cerial() {}
    void begin(int n) {}
    void println(char *s) {
        this->print(s);
        printf("\n");
    }
    void println() { printf("\n"); }
    void println(int n, int type=CHAR) {
        this->print(n, type);
        printf("\n");
    }
    void print(char *s) {
        printf("%s", s);
    }
    void print(char c) {
        print(c, CHAR);
    }
    void print(int n, int type) {
        if (type == CHAR) {
            printf("%c", n);
        } else if (type == DEC) {
            printf("%d", n);
        } else if (type == HEX) {
            printf("%x", n);
        } else {
            fprintf(stderr, "Type %d unknown\n", type);
        }
    }
    void write(char b) {
        fprintf(stderr, "[BYTE] 0x%02x\n", b&0xff);
    }
    int write(uint8_t *arr, int n) {
        for (int i=0; i<n; i++) {
            fprintf(stderr, "[BYTE-%d] 0x%02x\n", i, arr[i]&0xff);
        }
        return n;
    }
};

Cerial Serial, softSerial;

static time_t t0 = NULL;
int millis() {
    if (!t0) {
        t0 = time(NULL);
    }
    return (int) (time(NULL) - t0);
}

void delay(int ms) {
    // https://stackoverflow.com/a/28827188
    /*
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    if(nanosleep(&ts, NULL)) {
        perror("nanosleep");
    }
    */
    if (ms < 1000) {
        ms = 1;
    } else {
        ms /= 1000;
    }
    sleep(ms);
}

#define debugPrint(s) Serial.print(s);
#define debugPrintln(s) Serial.println(s);

// </Portado a C>

// Código de arduino_nodo_sensor
// https://github.com/niclabs/water-monitoring/blob/1d783c36ab93ba6df3f345133db620ecc9d2397b/energy/arduino_nodo_sensor/arduino_nodo_sensor.ino#L298
void sendBuffer() {
    /**
    Send the current data stored in the SensorPayload sp.
    **/
    sp.addChecksum();
    int buf_size = sp.getCurrentSize();
    //while(millis() - startTime < 1000); // Busy waiting used to wait one second for LoPy4 to awake.
    debugPrintln("Timer superado");
    int written = softSerial.write(sp.getBuffer(), buf_size);
    sp.resetBuffer();
    debugPrint(F("Length of data sent: "));
    debugPrintln(written);
}

//void setup() {
int main() {
  // <C>
  // </C>
  Serial.begin(9600);
  softSerial.begin(19200);

  Serial.println(F("Creating timestamps"));
  for (int i=0; i<N_READINGS; i++) {
    dummy_readings[i].ts = millis();
    printf("\nMILLIS: %d\n", millis());
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

      //softSerial.write(sp_buffer[i]);
    }
    Serial.println();
  }
  // Enviar el buffer al final (como los bloques de la SD)
  sendBuffer();
}

void loop() {}
