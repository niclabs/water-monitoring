# Librería PayloadDecoder

La siguiente librería decodifica una cadena de bytes proveniente de un nodo sensor

# Modo de uso

```python
import payloadDecoder as pd

base64_string = '8F+SrMMA8g==' #Input del decoder
readings = pd.decode_payload(base64_string)
print(readings)
```

## Funcionamiento

El decodificador asume la entrada de un string codificado en base64, representando una cadena de N bytes:

		b1b2b3...bN

el cual puede interpretarse como una secuencia de M unidades de senseo concatenadas de la forma:

		s1s2s3...sM

donde cada unidad de senseo está definida por los dos primeros bits que la componen. De esta manera, la decodificación se realiza con una determinada estrategia de decodificación, dependiendo de estos bits.

## Estructura general de un Sensing Unit o Unidad de Senseo:

La estructura a nivel de bytes de una unidad de Senseo es la siguiente:

| 1 byte      | 6 bytes   |
|-------------|-----------|
| Unit Header | Payload   |

donde el campo Payload sigue una estructura dependiendo de la información presente en el campo Unit Header. Por su parte, el campo Unit Header presenta la siguiente estructura en bits:

| 2 bits      | 2 bits    | 4 bits    | 
|-------------|-----------|-----------|
| Coding Info | Reserved  | Sensor ID |

* Coding Info: valor que determina el tipo de formato que tendrá el Payload adjunto
* Reserved: bits reservados, deben tener el valor 11 siempre
* Sensor ID: ID del sensor que realizó la medición

## Estrategias de decodificación

### Definición previa:

Llamaremos **lectura** a una tupla de tres elementos numéricos con la siguiente forma (sensor_id, timestamp, value). La lectura representa un valor medido por el sensor identificado por *sensor_id*, en el tiempo *timestamp* con una magnitud de *value*.

### Decodificación base

La decodificación se realiza cuando los dos primeros bits de una Sensing Unit son 11. La estructura a decodificar es la siguiente:

| 1 byte      | 4 bytes   | 2 bytes      | 1 byte      | 2 bytes       | 4 bytes          |
|-------------|-----------|--------------|-------------|---------------|------------------|
| Unit Header | Timestamp | Sensed Value | Unit Header | Time interval | Number of Values |

donde la sección Unit Header contiene los siguientes campos:

| 2 bits      | 2 bits    | 4 bits    | 
|-------------|-----------|-----------|
| Coding Info | Reserved  | Sensor ID |

siendo Coding Info la secuencia 11 en bits y Sensor ID el ID del sensor que realizó la lectura.

Luego de la decodificación, se entrega una lectura, con los valores respectivos presentes en la Sensing Unit.

### Decodificación repetida

La decodificación se realiza cuando los dos primeros bits de una Sensing Unit son 10. Cuando esto ocurre, se decodifican 14 bytes con la siguiente estructura:

| 1 byte      | 4 bytes   | 2 bytes      | 1 byte      | 2 bytes       | 4 bytes          |
|-------------|-----------|--------------|-------------|---------------|------------------|
| Unit Header | Timestamp | Sensed Value | Unit Header | Time interval | Number of Values |

cumpliéndose que ambos campos Unit Header almacenan igual Coding Info, con valor binario 10 e igual valor de Sensor ID.

Luego de la decodificación, se entrega una lista de lecturas, conteniendo un total de Number of Values + 1 lecturas en total. Las lecturas generadas consisten en la lectura presente en los primeros 7 bytes y las siguientes Number of Values lecturas se generan añadiendo una suma acumulativa de Time Interval al Timestamp de la primera lectura.

### Decodificación diferencial

Esta decodificación se realiza cuando los dos primeros bits de una Sensing Unit son 01. Cuando esto ocurre, se decodificarán por lo menos 14 bytes, agregando a la decodificación todas las Sensing Units que contengan 01 en su cabecera. La estructura a decodificar es la siguiente:

| 1 byte      | 4 bytes   | 2 bytes      | 1 byte      | 2 bytes        | 1 byte     | 2 bytes        | 1 byte     |...
|-------------|-----------|--------------|-------------|----------------|------------|----------------|------------|...
| Unit Header | Timestamp | Sensed Value | Unit Header | Dif Timestamp1 | Dif Value1 | Dif Timestamp2 | Dif Value2 |...

Luego de la decodificación, se entrega una lista con lecturas. El número de lecturas será igual a (N-7)*2, siendo N el número total de bytes decodificados. Las lecturas dentro del arreglo corresponderán a la lectura alojada en los primeros 7 bytes, la cual será la lectura de referencia, y las lecturas generadas al sumar los valores Dif Timestamp y Dif Value a la lectura de referencia.




## Tabla de sensores

| Sensor                                           | Variable a medir     | Sensor ID | Rango          | Error  | Resolución                |
|--------------------------------------------------|----------------------|-----------|----------------|---------|---------------------------|
| DS18B20                                          | Temperatura          | 0000      |-10 - 85 °C     | ±0.5°C  | 9 bit: 0.5 °C, 10 bit: 0.25°C, 12 bit: 0.0625 °C |
| SHT20 I2C                                        | Temperatura          | 0001      |-40 - 125 °C    | 0.3%    | 14 bit: 0.01 °C           |
| Gravity: Analog Water Pressure Sensor            | Presión              | 0010      | 0 - 1.6 MPa    | 0.7%    | |
| Pressure Sensor HK1100C                          | Presión              | 0011      | 0 - 1.2 MPa    | 1.5%    | |
| Gravity: Analog pH Sensor/Meter Kit V2           | PH                   | 0100      | 0 - 14         | ±0.1    | |
| Gravity: Analog Electrical Conductivity Meter    | Electroconductividad | 0101      | 10 - 100 mS/cm | ±5%     | |
| Gravity: Analog TDS Sensor/Meter for Arduino     | Electroconductividad | 0110      | 0 - 1000 ppm   | ±10%    | |
| Grove: Turbidity Sensor (Meter) for Arduino V1.0 | Turbidez             | 0111      |                |         | |
| BMP280 Barometric Pressure Sensor                | Presión              | 1000      | 0.3 - 1.1 MPa  | ±120 Pa | |

## Debugging
El módulo `payloadVerboseDecoder` provee la misma funcionalidad que `payloadDecoder`, pero mostrando la información de cada paquete de datos decodificado.
Para utilizarlo, basta reemplazar la línea
```python
import payloadDecoder as pd
```
por
```python
import payloadVerboseDecoder as pd
```

