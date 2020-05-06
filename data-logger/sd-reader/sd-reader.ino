#include <SPI.h>
#include <SD.h>
/*
El código considera la existencia de un botón que gatilla la escritura
a la tarjeta SD.
*/
#define CHIPSEL_PIN (53)
#define BUTTON_PIN (2)
File outFile;
String outFileName = "data.txt";


void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  Serial.print("Leyendo tarjeta SD del pin ");
  Serial.println(CHIPSEL_PIN);
  if(!SD.begin(CHIPSEL_PIN)){
    Serial.println("No se pudo detectar tarjeta SD.");
    while(1);
  }
  Serial.println("SD encontrada");
}

void loop() {
    if(digitalRead(BUTTON_PIN) == HIGH) {
      Serial.println("Boton presionado");
      outFile = SD.open(outFileName, FILE_WRITE);
      if(outFile){
        Serial.print("Escribiendo datos en ");
        Serial.println(outFileName);
        String dummyDate = "01/01/2020";
        float dummyLecture = 3.14159;
        outFile.print(dummyDate);
        outFile.print(",");
        outFile.println(dummyLecture);
        outFile.close();
        Serial.println("Listo");
      } else {
        Serial.println("No se pudo abrir el archivo en la SD");
      }
      delay(1000);
    }
}
