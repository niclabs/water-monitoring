# Sketches Experimentos Sensores - Iteración 4

Experimentos de la <b>Iteración 4</b> corresponden a más experimentos de Presión y Conductividad. Se está evaluando nuevos experimentos de sensores de pH, junto con buscar nuevos modelos o equipos de medición para todos lo sensores anteriores.

Experimentos se desarrollan en casa de Pablo Martín y en Laguna Carén.

## Programas Arduino

### FONDEF_carcasa_arduino_datalogger

Programa para electrónica a sumergir en Laguna Carén dentro de carcasa fabricada por +D.

| Funcionamiento |
| - |
| - Lectura periódica de sensor de RTC, Sensor de Presión y BME280, almacenamiento en micro SD
- Carga de batería 18650 de 3200mAh desde alimentación externa |

| Componentes | | |
| - |  - | - |
| Arduino Mini Pro 3.3V 8MHz | Módulo SD | DS3231 |
| Batería 18650 |  TP4056 | ADS1015 |

| Sensores | |
| - | - |
| Presión | BME280 |

### FONDEF_drifting_arduino_datalogger_it4_serial2nodemcu

Programa para Iteración 4 de experimentos de presión y conductividad. Corresponde al sketch de arduino que captura periódicamente datos desde sensores y rtc, los guarda en micro SD y los envía por serial para que NodeMCU los suba a Servidor Niclabs y Telegram Bot.


### FONDEF_drifting_arduino_datalogger_it4_serial2nodemcu_v2

Igual al anterior (_FONDEF_drifting_arduino_datalogger_it4_serial2nodemcu_) pero con dos ADS1015 para medir 6 sensores en paralelo (capacidad de 1 - 8 sensores análogos).

### FONDEF_drifting_nodemcu_HTTP_POST_niclabs

Programa para Iteración 4 de experimentos de presión y conductividad. Corresponde al sketch del NodeMCU (ESP8266) que toma datos desde arduino por puerto serial y los envía a Servidor Niclabs y Telegram Bot.
