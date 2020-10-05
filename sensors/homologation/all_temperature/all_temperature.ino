#include <OneWire.h>
#include <DallasTemperature.h> // DS18B20
#include <Wire.h> // SHT20

#define PRINT_NO_OF_DECIMALS 3
#define COL_SEP "; "
#define USE_SHT20 0 // SHT20 is not waterproof!
// ------------------------- TEMPERATURE -------------------------

// DS18B20
#define TEMP_DS18B20_PIN 5
#define DS18B20_BIT_RESOLUTION 9
OneWire onewire_bus(TEMP_DS18B20_PIN);
DallasTemperature temp_ds18b20(&onewire_bus);

uint8_t ds18b20_addrs[][8] = {{0x28, 0x55, 0x9E, 0x0E, 0x21, 0x19, 0x01, 0xE4},
                              {0x28, 0x53, 0x8B, 0xB0, 0x1B, 0x19, 0x01, 0xC8},
                              {0x28, 0x51, 0xA5, 0x56, 0xB5, 0x01, 0x3C, 0x67},
                              {0x28, 0xF0, 0x58, 0x56, 0xB5, 0x01, 0x3C, 0x46},
                              {0x28, 0x6B, 0xA6, 0x56, 0xB5, 0x01, 0x3C, 0x0B}};

// SHT20
#include <Wire.h>
#if USE_SHT20
#define I2C_MUX_PIN 4
#define MUX_SHT20_1 HIGH // Only this device is in this bus
#define MUX_SHT20_2 LOW // Main bus is here
#endif //USE_SHT20

// Thermistor
#define THERMISTOR_PIN A0
// RT: Thermistor resistance [Ohm] = M*( Temp [°C] ) + N
// RA: Auxiliary resistor
#define M_THERMISTOR -287.59921
#define N_THERMISTOR 17429.89389
#define RA_THERMISTOR 10000 // 10 KOhm
#define VCC_THERMISTOR 5 // V
/*
 *   VCC
 *    |
 *    RT
 *    ├---> Vin (to Arduino)
 *    RA
 *    |
 *   GND
 **/
float measure_thermistor(float vin) {
    // Equation derived from voltage divider above
    return (1/M_THERMISTOR)*( RA_THERMISTOR*(VCC_THERMISTOR-vin)/vin - N_THERMISTOR );
}

// Temp Atlas
//#define TEMP_ATLAS_I2C_ADDR 102
//#define TEMP_ATLAS_WAIT     600
#include <SoftwareSerial.h>
#define TEMP_ATLAS_SERIAL_RX 2
#define TEMP_ATLAS_SERIAL_TX 3
SoftwareSerial temp_atlas(TEMP_ATLAS_SERIAL_RX, TEMP_ATLAS_SERIAL_TX);
String temp_atlas_string = "";

// general
String input_str;
unsigned long t0;

void setup() {
    Serial.begin(9600);

    // Temp
    temp_ds18b20.begin();
    temp_ds18b20.setWaitForConversion(false); // <- Must wait manually! (bitResolution = 9 for OneWire)

#if USE_SHT20
    pinMode(I2C_MUX_PIN, OUTPUT);
    digitalWrite(I2C_MUX_PIN, MUX_SHT20_2); // default I2C bus

    Wire.begin();
    Serial.println(F("[TEMP] Will init SHT20 1..."));
    sht20_begin(&Wire);

    delay(10);
    Serial.println(F("[TEMP] SHT20 2..."));
    digitalWrite(I2C_MUX_PIN, MUX_SHT20_1);
    sht20_begin(&Wire);
    digitalWrite(I2C_MUX_PIN, MUX_SHT20_2);
#endif //USE_SHT20

    // Thermistor
    pinMode(THERMISTOR_PIN, INPUT);

    // (Atlas only need I2C)
    temp_atlas.begin(9600);
    temp_atlas_string.reserve(20);
    // Temp Atlas Serial doesn't respond first two measurements
    for (int i=0; i<2; i++) {
      temp_atlas.print("r");
      temp_atlas.print('\r');
      while (temp_atlas.available() > 0) {
        temp_atlas.read();
      }
    }
    // Make sure *OK echo is turned off
    temp_atlas.print("*OK,0");
    temp_atlas.print('\r');
    while (temp_atlas.available() > 0) {
      temp_atlas.read();
    }

    Serial.println(F("[Temp done]"));

    delay(500); 
    Serial.println(F("Setup done."));
}

void loop() {
  measuringMenu();
}

// -------------------- MEASURING --------------------

void measuringMenu() {
    while (1) {
        Serial.println(F("\nMEASURING menu:"));
        Serial.println(F("[1] All temperature sensors"));
        while (!Serial.available());
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '1':
                measuringInterface(measureTemp);
                return;
            default:
                Serial.println(F("[Not an option]"));
                continue;
        }

    }
}

void measuringInterface(void (*measuringCallback)(int)) {
    int index = 1;
    Serial.println(F("[0 to stop, anything else to take one measurement]"));
    while (1) {
        while (!Serial.available());
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '0':
                return;
            default:
                measuringCallback(index);
                index++;
        }
    }
}

// ------------------------- TEMPERATURE -------------------------

void measureTemp(int index) {
    if (index == 1) {
#if USE_SHT20
        Serial.println("#" COL_SEP "t" COL_SEP "DS18B20.1" COL_SEP "DS18B20.2" COL_SEP "DS18B20.3" COL_SEP "DS18B20.4" COL_SEP "DS18B20.5"
                           COL_SEP "SHT20.1" COL_SEP "SHT20.2" COL_SEP "Atlas"); // CSV Header
#else
        Serial.println("#" COL_SEP "t" COL_SEP "DS18B20.1" COL_SEP "DS18B20.2" COL_SEP "DS18B20.3" COL_SEP "DS18B20.4" COL_SEP "DS18B20.5"
                           COL_SEP "termistor" COL_SEP "Atlas"); // CSV Header
#endif //USE_SHT20
        if (temp_ds18b20.getDeviceCount() != 5) {
            Serial.println("[Error: expected 5 DS18B20 sensors]");
            return;
        }
        t0 = 0; // reset time
    }

    Serial.print(index, DEC);
    Serial.print(COL_SEP);

    Serial.print(millis() - t0, DEC);
    Serial.print(COL_SEP);
    for (int i = 0; i < 5; i++) {
        temp_ds18b20.requestTemperaturesByAddress(ds18b20_addrs[i]); 
    }

    temp_ds18b20.blockTillConversionComplete(DS18B20_BIT_RESOLUTION); // Modify DallasTemperature.h, move `blockTillConversionComplete` to public:

    float temp;
    // DS18B20
    for (int i = 0; i < 5; i++) {
        temp = temp_ds18b20.getTempC(ds18b20_addrs[i]);
        Serial.print(temp, PRINT_NO_OF_DECIMALS);
        Serial.print(COL_SEP);
    }

#if USE_SHT20
    // SHT20.1
    digitalWrite(I2C_MUX_PIN, MUX_SHT20_1); // Change mux selector
    temp = sht20_readTemperature(&Wire);
    Serial.print(temp, PRINT_NO_OF_DECIMALS);
    Serial.print(COL_SEP);
    digitalWrite(I2C_MUX_PIN, MUX_SHT20_2); // Back to normal bus

    delay(1);

    // SHT20.2
    temp = sht20_readTemperature(&Wire);
    Serial.print(temp, PRINT_NO_OF_DECIMALS);
    Serial.print(COL_SEP);
#endif //USE_SHT20

    // Termistor
    temp = measure_thermistor(5*analogRead(THERMISTOR_PIN)/1024.0);
    Serial.print(temp, PRINT_NO_OF_DECIMALS);
    Serial.print(COL_SEP);

    // Atlas
/*  // (I2C now unused, see below for Serial)
    Wire.beginTransmission(TEMP_ATLAS_I2C_ADDR);
    Wire.write("r");
    Wire.endTransmission();9.250;
    delay(TEMP_ATLAS_WAIT);
    Wire.requestFrom(TEMP_ATLAS_I2C_ADDR, 20, true);
    switch (Wire.read()) { // first byte is response code
        case 2:
        case 254:
        case 255:
            Serial.println("[Temp. Atlas failed]");
            return;
        case 1:
            break;
    }
    char buf;
    while (Wire.available()) {
        buf = Wire.read(); // Atlas sends value as text
        if (buf == '\0') {
            break; // ready
        }
        Serial.print(buf); // print digit
    }
*/

    temp_atlas.print("r");
    temp_atlas.print('\r');
    while (temp_atlas.available() > 0) {
      char inchar = (char)temp_atlas.read();
      if (inchar == '\r') {
        break;
      }
      temp_atlas_string += inchar;
    }
    //temp = temp_atlas_string.toFloat();
    //Serial.print(temp, PRINT_NO_OF_DECIMALS);
    Serial.print(temp_atlas_string);
    temp_atlas_string = "";


    Serial.println();
}

#if USE_SHT20
//// SHT20
//// Source: https://github.com/DFRobot/DFRobot_SHT20/blob/master/DFRobot_SHT20.h (and .cpp)
#define SHT20_I2C_ADDRESS                         0x40
#define SHT20_TRIGGER_TEMP_MEASURE_NOHOLD           0xF3
#define SHT20_ERROR_I2C_TIMEOUT                     998
#define SHT20_ERROR_BAD_CRC                         999
#define SHT20_READ_USER_REG                         0xE7
#define SHT20_MAX_WAIT                              100
#define SHT20_DELAY_INTERVAL                        10
#define SHT20_MAX_COUNTER                           (SHT20_MAX_WAIT/SHT20_DELAY_INTERVAL)

byte sht20_readUserRegister(TwoWire *wire) {
    byte userRegister;
    wire->beginTransmission(SHT20_I2C_ADDRESS);
    wire->write(SHT20_READ_USER_REG);
    wire->endTransmission();
    wire->requestFrom(SHT20_I2C_ADDRESS, 1);
    userRegister = wire->read();
    return (userRegister);
}

void sht20_showReslut(const char *prefix, int val) {
    Serial.print(prefix);
    if (val){
        Serial.println("yes");
    } else{
        Serial.println("no");
    }
}

#define SHT20_USER_REGISTER_END_OF_BATTERY          0x40
#define SHT20_USER_REGISTER_HEATER_ENABLED          0x04
#define SHT20_USER_REGISTER_DISABLE_OTP_RELOAD      0x02
void sht20_begin(TwoWire *wire) {
    byte reg = sht20_readUserRegister(wire);
    sht20_showReslut("End of battery: ", reg & SHT20_USER_REGISTER_END_OF_BATTERY);
    sht20_showReslut("Heater enabled: ", reg & SHT20_USER_REGISTER_HEATER_ENABLED);
    sht20_showReslut("Disable OTP reload: ", reg & SHT20_USER_REGISTER_DISABLE_OTP_RELOAD);
}

uint16_t sht20_readValue(TwoWire *wire, byte cmd) {
    wire->beginTransmission(SHT20_I2C_ADDRESS);
    wire->write(cmd);
    wire->endTransmission();
    byte toRead;
    byte counter;
    for(counter = 0, toRead = 0 ; counter < SHT20_MAX_COUNTER && toRead != 3; counter++){
        delay(SHT20_DELAY_INTERVAL);
        toRead = wire->requestFrom(SHT20_I2C_ADDRESS, 3);
    }
    if(counter == SHT20_MAX_COUNTER){
        return (SHT20_ERROR_I2C_TIMEOUT);
    }
    byte msb, lsb, checksum;
    msb = wire->read();
    lsb = wire->read();
    checksum = wire->read();
    uint16_t rawValue = ((uint16_t) msb << 8) | (uint16_t) lsb;
    if(sht20_checkCRC(rawValue, checksum) != 0){
        return (SHT20_ERROR_BAD_CRC);
    }
    return rawValue & 0xFFFC;
}

float sht20_readTemperature(TwoWire *wire) {
    uint16_t rawTemperature = sht20_readValue(wire, SHT20_TRIGGER_TEMP_MEASURE_NOHOLD);
    if(rawTemperature == SHT20_ERROR_I2C_TIMEOUT || rawTemperature == SHT20_ERROR_BAD_CRC){
        return(rawTemperature);
    }
    float tempTemperature = rawTemperature * (175.72 / 65536.0);
    float realTemperature = tempTemperature - 46.85;
    return (realTemperature);
}

#define SHT20_SHIFTED_DIVISOR                       0x988000
byte sht20_checkCRC(uint16_t message_from_sensor, uint8_t check_value_from_sensor) {
    uint32_t remainder = (uint32_t)message_from_sensor << 8;
    remainder |= check_value_from_sensor;
    uint32_t divsor = (uint32_t)SHT20_SHIFTED_DIVISOR;
    for(int i = 0 ; i < 16 ; i++){
        if(remainder & (uint32_t)1 << (23 - i)){
            remainder ^= divsor;
        }
        divsor >>= 1;
    }
    return (byte)remainder;
}

#endif //USE_SHT20
