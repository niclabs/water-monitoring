#include <OneWire.h>
#include <DallasTemperature.h> // DS18B20
#include <Wire.h> // SHT20

#define COL_SEP "; "
#define PRINT_NO_OF_DECIMALS 3

// ----------------------------- pH -----------------------------

// pHGravity.1
#include <DFRobot_PH.h>
#define PH_GRAVITY_PIN_1 A0
DFRobot_PH phGravity_1(1); // Don't start at 0! (DFRobot_EC uses those EEPROM addresses)

// pHGravity.2
#define PH_GRAVITY_PIN_2 A1
DFRobot_PH phGravity_2(2);

// pH Atlas
/*
#define PH_ATLAS_I2C_ADDR 99
#define PH_ATLAS_WAIT     815
*/
#include <SoftwareSerial.h>
#define PH_ATLAS_SERIAL_RX 2
#define PH_ATLAS_SERIAL_TX 3
SoftwareSerial ph_atlas(PH_ATLAS_SERIAL_RX, PH_ATLAS_SERIAL_TX);
String ph_atlas_string = "";


// general
String input_str;
char str_buf[20];
unsigned long t0;

void setup() {
    Serial.begin(9600);

    // pH
    pinMode(PH_GRAVITY_PIN_1, INPUT);
    pinMode(PH_GRAVITY_PIN_2, INPUT);
    phGravity_1.begin();
    phGravity_2.begin();

    ph_atlas.begin(9600); // Atlas connected
    ph_atlas_string.reserve(20);
    // Not sure if Atlas Serial responds first two measurements
    for (int i=0; i<2; i++) {
      ph_atlas.print("r");
      ph_atlas.print('\r');
      while (ph_atlas.available() > 0) {
        ph_atlas.read();
      }
    }
    // Make sure *OK echo is turned off
    ph_atlas.print("*OK,0");
    ph_atlas.print('\r');
    while (ph_atlas.available() > 0) {
      ph_atlas.read();
    }
    Serial.println("[pH done]");

    Serial.println(F("Setup done."));
}

void loop() {
  mainMenu();
}


void mainMenu() {
    while (1) {
        // this loop should never end
        Serial.println("MAIN menu:");
        Serial.println("[1] Measure");
        while (!Serial.available()); // Busy-wait for user input
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '1':
                measuringMenu();
                break;
            default:
                Serial.println("[Not an option]\n");
                continue;
        }
    }
}

// -------------------- MEASURING --------------------

void measuringMenu() {
    while (1) {
        Serial.println(F("\nMEASURING menu:"));
        Serial.println(F("[1] All pH sensors"));
        while (!Serial.available());
        input_str = Serial.readString();
        switch (input_str.charAt(0)) {
            case '1':
                measuringInterface(measurepH);
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

// ----------------------------- pH -----------------------------

void measurepH(int index) {
    if (index == 1) {
        Serial.println("#" COL_SEP "t" COL_SEP "pHGravity.1" COL_SEP "pHGravity.2" COL_SEP "Atlas"); // CSV Header
        t0 = 0; // reset time
    }
    Serial.print(index, DEC);
    Serial.print(";");

    Serial.print(millis() - t0, DEC);
    Serial.print(";");
    
    float voltage, ph;
    
    // pHGravity.1
    voltage = analogRead(PH_GRAVITY_PIN_1)/1024.0*5000;
    ph = phGravity_1.readPH(voltage, NULL);
    Serial.print(ph, PRINT_NO_OF_DECIMALS);
    Serial.print(";");

    // pHGravity.2
    voltage = analogRead(PH_GRAVITY_PIN_2)/1024.0*5000;
    ph = phGravity_2.readPH(voltage, NULL);
    Serial.print(ph, PRINT_NO_OF_DECIMALS);
    Serial.print(";");

    // pH Atlas
    /*
    Wire.beginTransmission(PH_ATLAS_I2C_ADDR);
    Wire.write("r");
    Wire.endTransmission();
    delay(PH_ATLAS_WAIT);
    Wire.requestFrom(PH_ATLAS_I2C_ADDR, 20, true);
    switch (Wire.read()) { // first byte is response code
        case 2:
        case 254:
        case 255:
            Serial.println("[pH Atlas failed]");
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
    ph_atlas.print("r");
    ph_atlas.print('\r');
    while (ph_atlas.available() > 0) {
      char inchar = (char)ph_atlas.read();
      if (inchar == '\r') {
        break;
      }
      ph_atlas_string += inchar;
    }
    //temp = ph_atlas_string.toFloat();
    //Serial.print(temp, PRINT_NO_OF_DECIMALS);
    Serial.print(ph_atlas_string);
    ph_atlas_string = "";

    Serial.println();
}
