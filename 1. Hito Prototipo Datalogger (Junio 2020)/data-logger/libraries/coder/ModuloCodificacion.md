# Librería SensorPayload

La librería SensorPayload para Arduino codifica las lecturas de los sensores de los datalogger en arreglos de bytes para ser transmitidos. Las lecturas se empaquetan en Sensing Units y luego se concatenan, generando un payload final con la siguiente estructura:

        SensingUnit1 SensingUnit2 SensingUnit3 ... SensingUnitN

## Sensing Unit

Cada Sensing Unit se conforma como sigue:

| 1 byte      | 6 Bytes   |
|-------------|-----------|
| Unit Header | Payload   |

Donde la sección Unit Header está conformada de la siguiente forma:

| 2 bits      | 2 bits    | 4 bits    | 
|-------------|-----------|-----------|
| Coding Info | Reserved  | Sensor ID |

### Coding Info

El campo Coding Info determina la estructura del payload de la Sensing Unit.

| Tipo de Payload     | Coding Info |
|---------------------|-------------|
| Payload Base        | 11          |
| Payload Repetido    | 10          |
| Payload Diferencial | 01          |

### Sensor ID 

La siguiente tabla determina los ID asociados a los distintos tipos de sensores

| Sensor                                           | Variable a medir     | Sensor ID |
|--------------------------------------------------|----------------------|-----------|
| DS18B20                                          | Temperatura          | 0000      |
| SHT20 I2C                                        | Temperatura          | 0001      |
| Gravity: Analog Water Pressure Sensor            | Presión              | 0010      |
| Pressure Sensor HK1100C                          | Presión              | 0011      |
| Gravity: Analog pH Sensor/Meter Kit V2           | PH                   | 0100      |
| Gravity: Analog Electrical Conductivity Meter    | Electroconductividad | 0101      |
| Gravity: Analog TDS Sensor/Meter for Arduino     | Electroconductividad | 0110      |
| Grove: Turbidity Sensor (Meter) for Arduino V1.0 | Turbidez             | 0111      |

## Payload Base

La estructura del Payload Base considera 6 bytes en total, divididos en dos secciones:

| 4 bytes   | 2 bytes      |
|-----------|--------------|
| Timestamp | Sensed Value |

* Timestamp: tiempo Unix que determina la fecha y hora, con precisión de segundos, donde se realizó una medida.
* Sensed Value: valor entero con signo entre -32767 y 32767 que representa un número real con precisión de un decimal. Así por ejemplo, el número 2354 se interpreta como 235.4

## Payload Repetido

Un Payload Repetido se genera cuando existen tres o más lecturas de datos sucesivas que reportan un mismo valor de senseo. Cuando esto ocurre, se codifica una primera Sensing Unit con Payload Base, pero con cabecera de Payload Repetido (Coding Info = 10). A continuación le procede un Sensing Unit con cabecera de Payload Repetido y un payload de 6 bytes con la siguiente estructura:

| 2 bytes       | 4 bytes          |
|---------------|------------------|
| Time interval | Number of Values |

* Time interval: Intervalo de tiempo, en segundos, entre las medidas repetidas.
* Number of values: Cantidad de veces que debe repetirse el valor repetido del Sensing Unit que le antecede.

De esta manera, para la transmisión de un Payload Repetido, serán necesarios 14 bytes contiguos que sigan la siguiente estructura:

| 1 byte      | 4 bytes   | 2 bytes      | 1 byte      | 2 bytes       | 4 bytes          |
|-------------|-----------|--------------|-------------|---------------|------------------|
| Unit Header | Timestamp | Sensed Value | Unit Header | Time interval | Number of Values |
^                                        ^
10                                       10

## Payload Diferencial

Un Payload Diferencial es una estructura que representa paquetes de datos a partir de diferencias numéricas con respecto a un Payload Base de referencia. Para clarificar esto, se define un conjunto de tres muestras para un mismo sensor con los siguientes valores:

        Muestras = [(sensor: 1, timestamp: 333333333, value: 27.8),
                    (sensor: 1, timestamp: 333333345, value: 29.3),
                    (sensor: 1, timestamp: 333333357, value: 27.5)]

Las últimas dos muestras pueden representarse de manera diferencial con respecto a la primera muestra, mostrando solamente las diferencias numéricas de los valores alojados en los campos timestamp y value, quedando la nueva representacion de la siguiente manera:

        Muestras_Dif = [(sensor: 1, timestamp: 333333333, value: 27.8),
                        (sensor: 1, d_timestamp: +12, d_value: +1.5),
                        (sensor: 1, d_timestamp: +24, d_value: -0.3)]

Considerando lo anterior, se define un Payload Diferencial de 6 bytes con la siguiente estructura:

| 2 bytes        | 1 byte     | 2 bytes        | 1 byte     |
|----------------|------------|----------------|------------|
| Dif Timestamp1 | Dif Value1 | Dif Timestamp2 | Dif Value2 |

* Dif Timestamp: valor entre 0 y 65535, representando la diferencia en segundos con respecto a un Payload Base de referencia. La codificación asume que las muestras codificadas están en orden ascendente con respecto al timestamp de cada una.
* Dif Value: valor con signo, entre 127 y -127 que representa la diferencia del valor sensado con respecto a un Payload Base de referencia. El valor además debe interpretarse como un número real con un décimal de precisión. Por ejemplo, el valor 100 debe interpretarse como 10.0, el valor 10 como 1.0 y el valor 1 como 0.1.

Con lo anterior, la transmisión de un Payload Diferencial se efectuará mediante la siguiente estructura de Sensing Units

        SensingUnitBase SensingUnitDiferencial1 SensingUnitDiferencial2 ... SensingUnitDiferencialN

Donde el SensingUnitBase tendrá en su campo de cabecera Coding Info el valor 01, indicando así su calidad de Payload Base de Referencia para las SensingUnitDiferenciales que le siguen.

