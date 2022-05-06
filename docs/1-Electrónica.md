# 💻 Electrónica

## Diseño
El dispositivo de electrónica (nodo sensor) diseñado para el monitorear el agua en intervalos de tiempo de manera autónoma, esta basado en tomar las medidas con sensores para ser enviadas mediante comunicación serial a un sistema de comunicación para el monitoreo de estas características, este dispositivo tomará medidas de pH, Conductividad, turbidez, temperatura y presión.


<<<<<<< HEAD
*imagen sistema completo resaltando el nodo sensor*
=======
>>>>>>> 78322bf4c01d1bb6e2c7b2995cf6d015adb8539d

### Prototipo
Como prototipo  se realizó un circuito en Arduino para realizar pruebas con sensores, medidas y comunicación para obtener una versión funcional que pudiera enviar los datos. Este prototipo su funcionamiento principal se basa en dos estados principalmente Reposo (Sleep), estado en el cual se esta en bajo consumo y sin trabajo de meddidas y segundo activo que esta encargado de tomar las medidas de los sensores, guardar estos datos de manera codificada y enviarlos de manera serial con comunicación 485.

El funcionamiento básico se puede ver en el siguiente diagrama:

<img title="a title" alt="Alt text" src="images/diagrama_func_simple.png">

#### Selección de componentes.
Dado lo anterior los componentes que integran este prototipo inicial son los siguientes:

1. **Arduino Nano:** Microcontrolador principal con el programa principal.
1. **ADC:** Lectura de señales analógicas a digitales de los sensores.
1. **RTC:** Encargado de establecer las alarmas de medida y envío al microcontrolador para llevar a cabos estas tareas.   
1. **Modulo SD:** Para el guardado de los datos medidos de manera codificada.
1. **Sensores:** Temperatura, conductividad, pH, Presión y Turbidez. 
1. **RS485:** Para realizar una comunicación serial de gran alcance(>10m).

Destacar que estos sensores seleccionados se sometieron a una serie de pruebas para comprobar su rendimiento, ver sus limitaciones y desgaste en el tiempo explicado en mas destalle en el apartado de sensores.

Por otra parte la conexión entre estos componentes esta dado se manera simplificada en el diagrama a continuación:

<img title="a title" alt="Alt text" src="images/diagrama_bloque_simple.png">
<img title="a title" alt="Alt text" src="images/electronica_conexiones_prototipo.png" width="400px">

Donde los sensores TDS, pH y turbidez poseen su electrónica, el de presión entrega sus valores al ADC y el de temperatura mediante comunicación "OneWire" directamente al microcontrolador. El módulo SD se comunica con el microcontrolador a través de conexión "ISCP", en cambio el ADC posee una conexión I2C para tener las señales de los 4 sensores y por último la información de los sensores es enviada de manera serial al componente 485 y es enviada mediante los canales A y B. 

Como resultados iniciales se tiene un nodo sensor capaz de obtener medidas, guardar estos datos en una microSD y además enviar estos datos de manera serial siendo un prototipo inicial lo suficientemente útil para la realización de pruebas en profundidad y analizar las limitaciones de los sensores bajo condiciones en profundidad controlada.

<img title="a title" alt="Alt text" src="images/electronica_prototipo_armado.png" width="500px">

### PCB
Continuando con las mejoras del nodo sensor, y el prototipo funcionando de manera adecuada, si bien es un dispositivo apto para la realización de pruebas controladas, se busca un dispositivo más apto en terrenos reales, como profundidades de aguas de al menos 30 metros. Para ello se realizó una integración den PCB con un fabricante local de PCB y diseño de circuitos, pasando prototipo a una PCB con mejoras incluidas dadas principalmente en el sistema de energía y espacio:

1. **Fuente energética con baterías:** Agregar al nodo un circuito de energía que sea alimentado a través de baterías de litio 18650 con toda su electrónica para un correcto funcionamiento (Cargadores y elevadores).
2. **Distribución energética de componentes:** Dividir la fuente de energía en una activa (5V) que siempre entrega alimentación al sistema y una de reposo (5Vs) que se apaga cuando el sistema no esta midiendo o enviado datos para aquellos componentes que puedan ser apagados para reducir el consumo lo mayor posible.
3. **Reducción de espacio para Carcasa:** Optimizar el espacio creando un dispositivo con la electrónica más compacto para la utilización de la carcasa especial que fue diseñada especialmente para ser más apta al sumergirse.

<img title="a title" alt="Alt text" src="images/diagrama_alimentacion.png">

La PCB incluye toda la electrónica de sensores y las necesarias  como reguladores de voltaje (-5V,3V,-3V y 3.3V) para componentes y electrónica siendo una pieza completa que se insertan los sensores para tomar las medidas. Posee además una conexión FTDI para realizar la programación del microcontrolador y un interruptor para utilizar comunicación serial directa al microcontrolador en vez de la 485 para la realización de pruebas.

<img title="a title" alt="Alt text" src="images/PCBnombrada_v0.png">



#### Historial de Versiones

Debido a diversos tipo de situaciones como falta de pistas o distribuciones de los componentes en diferentes fuentes de energía (5V y 5Vs) o simple optimización de esta, se obtuvieron diferentes versiones de PCB como modificaciones diferenciadas principalmente en la distribución energética a modo mejora,  estas versiones y modificaciones están resumidas en la siguiente tabla.

|       PCB         |                  **Modificación**                 |                    **Características**                   |                         **Resultado**                        |                              **Problemas**                             |
|----------------|:--------------------------------------------|:--------------------------------------------------------|:------------------------------------------------------------|:----------------------------------------------------------------------|
|    **Versión 1.a**    | Primera PCB fabricada                        | Distribución de alimentación en reposo y continua.       | Placa completamente funcional                                | La alimentación de reposo solo esta conectada con el componente RS485. |
| Versión 1.b | Corte y cambio de pistas para la PCB inicial | Distribución energética de componentes mas eficiente.    | Reducción del consumo en estado activo.                      | Componente RS485 de alto consumo en reposo                               |
| Versión 1.c | Cambio de componente RS485 manualmente        | Optimización de consumo en reposo                         | Problema funcional de la placa, (mala practica de soldadura) | PCB sin funcionamiento. |
|    **Versión 2.a**    | PCB con enrunado nuevo                      | Componentes divididos en su mayoría correctamente         | Placa completamente funcional                                | RS485 de alto consumo y en alimentación continua, falta de una pista.  |
|    **Versión 3.a**    | PCB con enrunado nuevo                      | Corrección de la pista faltante de la versión anterior. | Placa completamente funcional                                | RS485 de alto consumo y en alimentación continua.                      |

Estas versiones se ejemplifican en esquemas a continuación para detallar los cambios de distribución energética de los componentes que son alimentados con 5Vs y 5v respectivamente.

<img title="a title" alt="Alt text" src="images\diagrama_versiones.png">

<br>
- Mejoras no realizadas: como mejoras no realizadas se tiene el cambio del componente de comunicación 485 de alto consumo debido a que es antiguo, aun más en esta versión ya que esta conectado en la alimentación constante por lo que este consume con el sistema en reposo.
## Fabricación

Con los archivos de una versión funcional del sistema (Versión N°2), se realizó un estudio en la fabricación de esta placa para tener una idea y obtener un análisis en términos de los costos que tiene finalmente el "nodo sensor". 


### Cotización:
Para cotizar se consideró la posibilidad de una fabricación y ensamblaje completamente externo con los componentes entregados por el fabricante y otra con un ensamblado local con los componentes obtenidos mediante distribuidores.

#### 1. Fabricación y ensamblaje externo:
Consideraciones:
 * Para esta cotización se tomaron en cuenta los fabricantes: JLCPCB, PCBway, PCBgogo, EEcart, SeedStudio.
 * Los precios obtenidos son dados en base a cotizaciones rápidas entregadas por los fabricantes.
 * Los archivos requeridos para las cotizaciones Gerber (PCB), BOM (componentes) con un formato específico para cada fabricante y el *"pick and place"* en algunos casos.
 * La cotización fue realizada para una cantidad de 5 PCBs ensambladas (cantidad mínima aceptada).

 #### 2. Fabricación externa y ensamblado local:
 Consideraciones:
* El ensamblaje se consideró de manera manual.
* Utilización de stencil para el ensamblado.
* Se realizó una cotización de 5 placas.
* No todos los componentes son fáciles de obtener, se tiene que buscar sustitutos dependiendo del stock.
* Digikey y Mouser como distribuidores de componentes (al ser los mas completos).
#### 3. Comparación de opciones:

Dado el detalle del valor de componentes se hace una comparación entre las 2 opciones de fabricado,se consideró EEcart y PCB como fabricantes por ser las mejores opciones. además de una fabricación completa externa con una optimización de precios en componentes (RTC,pH), por un lado el RTC tiene un precio elevado y por otro el conector de pH se adquiere con el sensor.

Resultados y precios:

|                             | **Total (5u)** | **Total+ 30%** | **Precio unitario** | **Tiempo**  |
|-----------------------------|----------------|----------------|---------------------|-------------|
| **Local**                   | $772.09        | $1003.72       | $200.74             | 3-4 semanas |
| **Externa EEcart**          | $907.26        | $1179.44       | $235.89             | 5 semanas   |
| **Externa PCBWay**          | $888.45        | $1176.19       | $231.00             | 5.5 semanas |
| **Externa PCBWay (RTC,pH)** | $705.06        | $916.58        | $183.32             | 5 semanas   |


Considerar además un precio extra en cuando a sensores y baterías del dispositivo de $100.92USD/unidad aprox.

#### 4. Pros y contras

|    Fabricación Local    | Fabricación Externa |
|-----------------------------|----------------|
| - Menos tiempo estimado de realización. <br> - Menor precio estimado. <br> - Ensamblado manual. <br> - Dificultad de obtención de algunos componentes (stock limitado).  |      - La placa llega para usar directamente. <br> - Margen de optimización. <br> - Mayor tiempo de espera. <br> - Mayor precio.       |
### Programación
Para el programa principal del dispositivo posee 2 versiones con y sin SD con un funcionamiento muy similar, en general, el código posee un tiempo de alarma de tomar medidas con los sensores y otra alarma para el envío de datos de manera serial, El dispositivo pasa de estar en reposo al realizar la tarea requerida en la alarma y volver a esta en reposo para un ahorro de consumo.

Por otra parte para un eficiente envío de datos las medidas realizadas con su fecha y sensor correspondiente están codificados de manera binaria para reducir el tiempo de envío, así hacer un envío más eficiente con más cantidad de datos en un menor tiempo reduciendo el consumo cuando el dispositivo esta activo. Para conocer mas detalles ver el apartado de codificación.

Destacar que el los programas utilizaron las librerías: ASD1X15 para el ADC, DallasTemperature y OneWire para el sensor de temperatura, RTClib para el RTC, SDfat para la SD y una librería propia para la codificación.

#### Uso y carga de código a la PCB.
Para utilizar la PCB se requiere un previo montaje para su uso de cual consta de los siguientes pasos para su programación.
1. Conectar los **sensores** a sus respectivos puertos.
2. Insertar la **pila del RTC** en su respectivo módulo.(Si no posee alimentación la fecha siempre se reiniciará).
3. Insertar la **tarjeta SD** en su ranura si se trabajará con ella.
4. La **alimentación** puede ser de manera **externa** con la conexión correspondiente en el final de la PCB o mediante **baterías de litio** en los pines designados con su polo correspondiente.

Para programar la placa esta debe estar **siempre alimentada**:

5. Dejar el **interruptor en FTDI**.
6. Conectar un **adaptador FTDI-USB** para su programación en el puerto FTDI y un adaptador RS485-USB si se realiza esta comunicación en la salida 485 de la PCB.
7. Conectar **solo** el adaptador USB-FTDI al computador para pruebas.

La programación se realiza en Arduino IDE:

8. Seleccionar **el puerto** de conexión correspondiente en la pestaña herramientas.
9. Seleccionar  **Arduino Uno** en la pestaña herramientas.
10. Cargar el código correspondiente.

Se puede verificar el funcionamiento del programa en el monitor serial de Arduino IDE (Pestaña herramientas, monitor serie). Para el caso de ver el funcionamiento en comunicación 485, luego de cargar el programa desconectar el adaptador FTDI y conectar el 485 al computador, cambiar el interruptor a 485 y el funcionamiento se verá en el monitor serie.

#### Características.
Para realizar cambios en el código o generar uno propio se tiene:
1. Considerar el dispositivo como un **Arduino Uno**.
2. Importante habilitar el pin D3(alimentación) para la iniciar la SD.

Conexiones:

3. El **pin D2** es entrada asociada a la señal de interrupción del **RTC**.
4. El **pin D3** es salida y habilita la **Alimentación 5Vs**.
5. El **pin D4** es salida y habilita el **RS485** para que trabaje (Receptor y Emisor).
6. El **pin D9** es entrada con la señal del sensor de **temperatura** (1-wire).
7. **PC4 y PC5** conexión I2C proveniente del **ADC** que contiene los sensores de presión, turbidez, conductividad y pH.
8. La **SD** tiene una conexión **ISCP (pin 10-13)**.




## Roadmap
backlog
pasos futuros

| N  | Título                                         | Detalle                                                                                                                                                                                                                                                                                                                                                                                                      |
| -- | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 1  | Mover uSD a superficie                         | Hacer que la uSD sea controlada y alimentada desde el nodo de comunicaciones en vez del nodo sensor.                                                                                                                                                                                                                                                                                                         |
| 2  | Bajar ciclos del oscilador a 8Mhz              | Disminuir ciclos del oscilador de 16MHz a 8MHz. Esto disminuye la velocidad a la que opera el sistema sin afectar el funcionamiento de los programas. Funcionando a la mitad de ciclos se espera una disminución del consumo del procesador en el sistema.                                                                                                                                                   |
| 3  | Ocupar VCC 3.3V                                | Bajar alimentación del sistema de 5V a 3.3V. Se debe prototipar y validar el funcionamiento de componentes y electrónica. Con este cambio disminuye el consumo general de energía del sistema obteniendo mayor desempeño de estados de deep-sleep.                                                                                                                                                           |
| 4  | Amplificador universal multiplexado            | Si se caracterizan debidamente las sondas y se verifica que el procesamiento de los circuitos intermediarios se pueden realizar de forma digital, una opción es tener un amplificador para todas las sondas, las cuales mediante multiplexión temporal entren a éste (opcionalmente un digipot puede ayudar a modificar la ganancia). Esto podría reducir drásticamente la cantidad de componentes en placa. |                                                                                                                                                                                                                                                                                                                                                                                                      |
| 5  | Autonomía de sensores (drift)                  | Si se va a aumentar la autonomía del sistema, es necesario verificar si en ese horizonte de tiempo los sensores serán funcionales.                                                                                                                                                                                                                                                                           |
| 6  | Optimización de componentes                    | Relacionado con los puntos 3 y 4, hay que repasar toda la selección de componentes en términos de funciones y equipos. También se relaciona con la programación.                                                                                                                                                                                                                                             |                                                                                                                                                                                                                                                            |
| 7  | Autonomía energética (solar, electrónica)      | Añadir un sistema de carga solar para recargar las baterías 18650 del nodo sensor podría en prácticamente cualquier caso mejorar la autonomía.                                                                                                                                                                                                                                                               |
| 8  | Batería de auto                                | Colocar una batería de auto para todo el sistema teóricamente aseguraría una mayor autonomía.                                                                                                                                                                                                                                                                                                                |
| 9  | Programar nodo a distancia                     | \- Se podría cambiar el atmega por un esp que se programe a distancia.                                                                                                                                                                                                                                                                                                                                       |
| 10 | Switch magnético de encendido                  | Se puede agregar un switching on/off del sistema que se puede activar desde fuera de la carcasa con un imán para de esta manera poder ahorrar energía cuando no se esté utilizando el dispositivo.                                                                                                                                                                                                           |
| 11 | Modularizar datalogger de adaptadores sensores | Separar en placas separadas, pero conectables el datalogger (uC, RTC, SD, energía) de electrónica de amplificación de los sensores dfrobot (pH, turbidez, conductividad)     