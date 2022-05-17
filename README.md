# Sensores 🌡 

<!--
TODO: poner modelo de sensores de referencia.
TODO: poner cosas que salieron mal: medicion presión a 40cm, ...)
TODO: poner fotos de todos los experimentos para todos.
TODO: terminar de analizar íultimos datos.
TODO: poner tabla de resuemn resultados sensores.
-->

Se seleccionaron los sensores en la siguiente tabla para medir las variables fisicoquímicas de: Temperatura, Conductividad, pH, Nivel de agua y Turbidez.

| Variable                   | Modelo Sensor                    | Rango                                       | Error                              | Resolución                       |
| -------------------------- | -------------------------------- | ------------------------------------------- | ---------------------------------- | -------------------------------- |
| Tº                         | DS18B20                          | \-10°C - 85°C                               | ±0.5 ºC                            | 0,0625 °C<br>(12 bit)            |
| Presión                    | HK1100C                          | 0 - 1,2 MPa                                 | 1.5% FS                            | 18 kPa<br>1.8 mt H2O             |
| pH                         | Gravity: Analog pH Sensor/Meter  | 0 - 14 pH                                   | ±0,1 pH                            | 0,014 pH<br>(12 bit)             |
| Conductividad<br>Eléctrica | Gravity: Analog TDS Sensor/Meter | 0 - 1000ppm<br>0 - 2000 uS/cm<br>0 - 2mS/cm | ± 10% FS<br>200 uS/cm<br>0.2 mS/cm | 1,3 ppm<br>2,6 uS/cm<br>(12 bit) |
| Turbiedad                  | Grove - Turbidity Sensor/Meter   | 0 - 3000 NTU                                | No indica                          | 4,5 NTU<br>(12 bit)              |

- además se toma en cuenta los voltajes de funcionamiento de los sensores y el protocolo de comunicación.


Por lo general se sometió a los sensores a tipos de pruebas que permitieran evaluar dos dimensiones principales de su comportamiento: (1) su presición y (2) su desempeño por periodos prolongados de sumersión en medio acuático ('sensor drift'). 

## Temperatura


<img title="a title" alt="Alt text" src="docs/images/sensor_temp.png" width="200px">
<img title="a title" alt="Alt text" src="docs/images/sensor_temp_experimento1_foto.jpg" width="300px">
<img title="a title" alt="Alt text" src="docs/images/sensor_temp_experimento1.png" width="300px">

Experimentos demostraron un buen desempeño del sensor en su comportamiento en el tiempo

- comportamiento súper predecible.
- se debe ajustar el offset inicial.
- después de un tiempo se puede observar una leve desviación de la medidas, pero se puede corregir completamente reajustando el offset.
- errores menores a 0.5ºC y 1ºC


TODO: - Colocar sensor de referencia para saber cuál es.

<img title="a title" alt="Alt text" src="docs/images/sensor_temp_plot_hist.png">

- Error de dos sensores ds18b20 el primer mes de medición:
TODO: (se puede corregir por offset el error 1? indicar que sensores tenían un poco de uso ya?)

<img title="a title" alt="Alt text" src="docs/images/sensor_temp_experimento1_error.png" width="400px">

- Error de sensores luego de 7 meses de uso. Experimento dura 1 mes.

<img title="a title" alt="Alt text" src="docs/images/sensor_temp_experimento3_error.png" width="400px">

- Se desarrolla nueva metodología hacia últimos experimentos: ver perfil de medición en el rango. Se observa algo de histéresis en ambos sensores para la bajada.
TODO: (considerar rápidez en la toma de datos, existe histéresis en la bajada de temperatura para cambios rápidos)


<img title="a title" alt="Alt text" src="docs/images/sensor_temp_experimento3_perfil_inicial.png" width="700px">
<img title="a title" alt="Alt text" src="docs/images/sensor_temp_experimento3_rmse_perfil.png" width="500px">

## Conductividad

<img title="a title" alt="Alt text" src="docs/images/sensor_tds.png" width="300px">

TODO: (fotos del experimento)
TODO: al medir dos sensores de conductividad se afectan la medida. Pero el de conductividad con pH o presión o temp no.

- Ojo: Documentación indica rango [0,2000 uS/cm] ([link producto](https://www.dfrobot.com/product-1662.html))

- Perfil de medición luego de 1 mes de experimento:
    - metodología incluye sensor de control.
    - desgaste irrecuperable del sensor sumergido.
    - no se ve histéresis y tampoco en los datos

<img title="a title" alt="Alt text" src="docs/images/sensor_tds_experimento3_perfil_ini.png" width="500px">

<img title="a title" alt="Alt text" src="docs/images/sensor_tds_experimento3_perfil_fin.png" width="500px">

- Si es importante la saturación sobre los valores ~1500 uS/cm y ~2000 uS/cm.

<img title="a title" alt="Alt text" src="docs/images/sensor_tds_experimento3_rmse_rango.png" width="500px">

- Se esperan más experimentos para ver si es por depositos de minerales en el sensor, o si es corregible, o fue un error puntual, o etc.

## pH

<img title="a title" alt="Alt text" src="docs/images/sensor_ph.jpeg" width="300px">

TODO: fotos experimento

- no se ve mucha histéresis
- hay harta variación en la calibración pero es posible recalibrarla y los sensores vuelven a funcionar.
- se esperar hacer más experimento para corroborar datos y además poder ir estimando que tan rápido se descalibran los datos, y cuánto se va perdiendo de presición con cada recalibración.

- Perfil inicial del experimento:
<img title="a title" alt="Alt text" src="docs/images/sensor_ph_experimento3_perfil_ini.png" width="500px">

- Perfil al final del experimento sin recalibrar:
<img title="a title" alt="Alt text" src="docs/images/sensor_ph_experimento3_perfil_int.png" width="500px">

- Perfil final con recalibración de los sensores:
<img title="a title" alt="Alt text" src="docs/images/sensor_ph_experimento3_perfil_fin.png" width="500px">


## Nivel de Agua

<img title="a title" alt="Alt text" src="docs/images/sensor_presion.png" width="200px">
<img title="a title" alt="Alt text" src="docs/images/sensor_presion_atm.png" width="200px">

- Se utilizan dos sensores para calcular la columna de agua 
- TODO: Poner sensores dereferencia para ambos casos.

<img title="a title" alt="Alt text" src="docs/images/sensor_presion_experimento1_0.png" width="300px">
<img title="a title" alt="Alt text" src="docs/images/sensor_presion_experimento1_1.jpeg" width="300px">
<img title="a title" alt="Alt text" src="docs/images/sensor_presion_experimento1_2.jpeg" width="300px">

<img title="a title" alt="Alt text" src="docs/images/sensor_presion_experimento1.png" width="1000px">
<img title="a title" alt="Alt text" src="docs/images/sensor_presion_experimento1_rmse.png" width="350px">

- últimos experimentos fueron en baldes de agua y mato, porque muy poca altura asi que el desempeño fue peor, pero este no empeoró luego de 6 meses con sensores sumergidos.

<img title="a title" alt="Alt text" src="docs/images/sensor_presion_experimento3_rmse.png" width="300px">

## turbidez

<img title="a title" alt="Alt text" src="docs/images/sensor_turbidez.png" width="200px">

- primeros experimento dieron cualquier cosa, dificultad en uso de sensor muy delicados a la luz y otras variables dificiles de controlar.

<img title="a title" alt="Alt text" src="docs/images/sensor_turbidez_experimento1.png" width="500px">

- Errores del rango de ~1000 NTU. No confiables, quizás solo apra alertas grandes pero mejor seguir buscando.

<img title="a title" alt="Alt text" src="docs/images/sensor_turbidez_experimento2_1.png" width="500px">



# 💻 Electrónica

## Diseño
El dispositivo de electronica (nodo sensor) diseñado para el monitoreo de de agua en intervalos de tiempo de manera autónoma, esta basado en tomar las medidas con sesores para ser enviadas mediante comunicación serial a un sistema de comunicación para el monitoreo de estas características, este dispositivo tomará medidas de pH, Condutividad, turbidez, temperatura y presión.

*imagen sistema completo resaltando el nodo sensor*

### Prototipo
Como prototipo  se realizó un circuito en Arduino para realizar pruebas con sesores, medidas y comunicación para obtener una versión funcional que pudiera enviar los datos. Este prototipo su funcionamiento principal se basa en dos estados principalmente Reposo (Sleep), estado en el cual se esta en bajo consumo y sin trabajo de meddidas y segundo activo que esta encargado de tomar las medidas de los sensores, guardar estos datos de manera codificada y enviarlos de manera serial con comunicación 485.

El funcinamiento básico se puede ver en el siguiente diagrama:

<img title="a title" alt="Alt text" src="docs/images/diagrama_func_simple.png">

#### Selección de componentes.
Dado lo anterior los componentes que integran este prototipo inicial son los siguientes:

1. **Arduino Nano:** Microcontrolador principal con el programa principal.
1. **ADC:** Lectura de señales analógicas a digitales de los sensores.
1. **RTC:** Encargado de establecer las alarmar de medida y envio al microcontrolaor para llevar a cabos estas tareas.   
1. **Modulo SD:** Para el guardado de los datos medidos de manera codificada.
1. **Sensores:** Temperatura, conducitividad, pH, Presión y Turbidez. 
1. **RS485:** Para realizar una comunicación serial de gran alcance(>10m).

Destacar que estos sensores seleccionados se sometieron a una serie de pruebas para comprobar su rendimiento, ver sus limitaciones y desgaste en el tiempo explicado en mas destalle en el apartado de sensores.

Por otra parte la conexión entre estos componentes esta dado se manera simplificada en el diagrama a continuación:

<img title="a title" alt="Alt text" src="docs/images/diagrama_bloque_simple.png">
<img title="a title" alt="Alt text" src="docs/images/electronica_conexiones_prototipo.png" width="400px">

Donde los sensores TDS, pH y turbidez poseen su electrónica, el de presión entrega sus valores al ADC y el de temperatura mediante comunicación "OneWire" directamente al microcontrolador. El módulo SD se comunica con el microcontrolador a travez de conexion "ISCP", en cambio el ADC posee una conexión I2C para tener las señales de los 4 sensores y por último la imformación de los sensores es enviada de manera serial al componente 485 y es enviada mediante los canales A y B. 

Como resultados iniciales se tiene un nodo sensor capaz de obtener medidas, guardar estos datos en una microSD y además enviar estos datos de manera serial siendo un prototipo inicial lo suficientemente útil para la realización de pruebas en profundidad y analizar las limitaciones de los sensores bajo condiciones en profundidad controlada.

<img title="a title" alt="Alt text" src="docs/images/electronica_prototipo_armado.png" width="500px">

### PCB
Continuando con las mejoras del del nodo sensor, y el prototipo funcionando de manera adecuada, si bien es un dispositivo apto para la realización de pruebas controladas, se busca un dispositivo más apto en terrenos reales, como profundidasdes de aguas de al menos 30 metros. Para ello se relizó una integración den PCB con un frabicante local de PCB y diseño de circuitos, pasando prototipo a una PCB con mejoras incluidas dadas principalmente en el sistema de energía y espacio:

1. **Fuente energética con baterías:** Agregar al nodo un circuito de energía que sea alimentado a travéz de baterías de litio 18650 con toda su electrónica para un correcto funcionamiento (Cagardores y elevadores).
2. **Distribución energética de componentes:** Dividir la fuente de energía en una activa (5V) que siempre entrega alimentación al sistema y una de reposo (5Vs) que se apaga cuando el sistema no esta midiendo o enviado datos para aquellos componentes que puedan ser apagados para reducir el consumo lo mayor posible.
3. **Reducción de espación para Carcasa:** Optimizar el espacio creando un dispositivo con la electrónica más compacto para la utilización de la carcasa especial que fue diseñanada especialemnte para ser más apta al surmergirse.

<img title="a title" alt="Alt text" src="docs/images/diagrama_alimentacion.png">

La PCB incluye toda la electrónica de sensores y las necesarias  como reguladores de voltaje (-5V,3V,-3V y 3.3V) para compoenetes y electrónica siendo una pieza completa que se insertan los sensores para tomar las medidas. Posee además una conexion FTDI para realizar la programación del microcontrolador y un interruptor para utilizar comunicación serial directa al microcontrolador en vez de la 485 para la realización de pruebas.

<img title="a title" alt="Alt text" src="docs/images/PCBnombrada_v0.png">



#### Historial de Versiones

Debido a diversos tipo de situaciones como falta de pistas o distribuciones de los componentes en diferentes fuentes de energía (5V y 5Vs) o simple optimización de esta, se obtuvieron diferentes versiones de PCB como modificaciones diferenciadas principalmente en la distribución energética a modo mejora,  estas versiones y modificaciones estan resumidas en la siguiente tabla.

|       PCB         |                  **Modificación**                 |                    **Características**                   |                         **Resultado**                        |                              **Problemas**                             |
|----------------|:--------------------------------------------|:--------------------------------------------------------|:------------------------------------------------------------|:----------------------------------------------------------------------|
|    **Versión 1.a**    | Primera PCB fabricada                        | Distribución de alimentación en reposo y continua.       | Placa completamente funcional                                | La alimentación de resposo solo esta conectada con el compoente RS485. |
| Version 1.b | Corte y cambio de pistas para la PCB inicial | Distribucion energética de compoenetes mas eficiente.    | Reducción del consumo en estado activo.                      | Componete RS485 de alto consumo en rposo                               |
| Version 1.c | Cambio de compoente RS485 manualmente        | Optimización de consumo en rposo                         | Problema funcional de la placa, (mala practica de soldadura) | PCB sin funcionamiento. |
|    **Versión 2.a**    | PCB con enrrutado nuevo                      | Compoentes divididos en su mayoria correctamente         | Placa completamente funcional                                | RS485 de alto consumo y en alimentación continua, falta de una pista.  |
|    **Versión 3.a**    | PCB con enrrutado nuevo                      | Correccion de la pista faltante de la version antrerior. | Placa completamente funcional                                | RS485 de alto consumo y en alimentación continua.                      |

Estas versiones se ejemplifican en esquemas a continuación para detallar los cambios de distribución energética de los compoentes que son alimentados con 5Vs y 5v respectivamente.

<img title="a title" alt="Alt text" src="docs/images/diagrama_versiones.png">

<br>
- Mejoras no realizadas: como mejoras no realizadas se tiene el cambio del componente de comunicación 485 de alto consumo debido a que es antiguo, aun más en esta versión ya que esta conectado en la alimentación constante por lo que este consume con el sistema en reposo.
## Fabricación

Con los archivos de una versión funcional del sistema (Versión N°2), se realizó un estudio en la fabricación de esta placa para tener una idea y obtener un analisis en terminos de los costos que tiene finalmente el "nodo sensor". 


### Cotización:
Para cotizar se considero la posibilidad de una fabricación y ensablaje completamente externo con los componetes entregados por el fabricante.

#### 1. Fabricación y ensamblaje externo:
Consireaciones:
 * Para esta cotización se tomaron en cuenta los fabricantes: JLCPCB, PCBway, PCBgogo, EEcart, SeedStudio.
 * Los precios obtenidos son dados en base a cotizaciones rapidas entregadas por los fabricantes.
 * Los archivos requeridos para las cotizaciones Gerber (PCB), BOM (componentes) con un formato específico para cada fabricante y el *"pick and place"* en algunos casos.
 * La cotización fue realizada para una cantidad de 5 PCBs ensambladas (cantidad mínima aceptada).

 #### 2. Fabircación externa y ensamblado local:
 La cotización de esta opción la placa es fabricada de manera externa y los componetes cotizados a los distribuidores.

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

|    Fabricación Local    | Fabricacion Externa |
|-----------------------------|----------------|
| - Menos tiempo estimado de realización. <br> - Menor precio estimado. <br> - Ensamblado manual. <br> - Dificultad de obtención de algunos componentes (stock limitado).  |      - La placa llega para usar directamente. <br> - Margen de optimización. <br> - Mayor tiempo de espera. <br> - Mayor precio.       |

### Programación
Hablando del software del dispositivo nodo sensor se poseen 2 versiones  con y sin SD para su funcionamiento, estos posee las siguentes caracteristicas:
1. Posee variables de los períodos de medida y envvío de datos: Es una de las características más importantes para la definición de tiempos de funcinamniento del sistema. La variable "Freq_sens" indica cada cuantos segundos tomada una medida de los sensores y la variable "Freq_send" representa el intervalo en segundos en el que son enviados los datos, claramente  Freq_send > Freq_sens  ya que deben existir datos medidos para ser enviados.
2. Codificación de lectura de datos: Para realizar un envío más eficiente ya sea por velocidad, memoria, disminución de consumo energético en el momento de enviar datos se desarrolló una librería que codifique los datos para ello. Lo importante a destacar es qu exite una codificación de lectura y otra de envio explicada mas adelante.

La codificación de lectura simplemente pasa a bytes(9) la medida (valor) del sensor, tiempo(timestamp) dado por la alarma de medida del RTC en formato "Unix" y si ID del sensor (valor predeterminado).

3. Guardado de datos en un "DataBlock": Los datos de cada intervalo, es decir, el tipo (sensor ID), su valor y tiempo de medida (Timestamp) en cada sensor son codificados (explicado más adelante) en bytes al que llamaremos "sensor reading", estos son generados en cada medida y será guardado en el "Datablock" (arreglo) de 512 bytes que actuara como memoria RAM-buffer. Los "sensor reading" son lo que poseen la codificación de lectura con un largo de 9 bytes para el guardado de los datos. Dado esto la estructura del Datablock esta dada por 2 bytes para llevar el conteo de medidas, 504 bytes para "sensor reading" (56 medidas u 11 medidas de 5 sensores) y 6 bytes sobrantes como se puede ver a continuación.

Para el caso de la sd cuando el DataBlock esta lleno guarda los datos en archivos binarios dentro de la tarjeta SD y los envía junto a los valores del arreglo cuando sea el tiempo de enviado con una codificación nueva para el enviado. Si es la versión sin sd no se posee más memoria que la del arreglo por lo que esta limitado a medir una cantidad maxima de datos antes de enviar, por lo que la "Freq_send" tiene esta limitante, por ejemplo,un máximo de 11 veces la Freq_sens para 5 sensores para no perder datos en la codificación base.

<img title="a title" alt="Alt text" src="docs/images/datablock.png">

4. Codificación par envío de datos: Para reducir la cantidad de tamaño de los datos enviados y asi realizar un envío más rápido con menor consumo, se reali´zaron distitos tipos de codificación de enviado a modo de reducir el tamaño de los datos.
* Codificiación de envío base:  Para esta codificiación y las siguientes se define un nuevo bloque "Sensing Unit", que se divide en 1byte "Header" que posee la informacion del tipo de codificación y la ID del sensor y 6 bytes "Payload" que contiene 4bytes para el Tiemstamp y solo 2bytes para el valor del sensor, esta reducción ene le valor del sensor se debe a que los valores de los sensores al ser pequeños pueden ser prefectamente representados en 2bytes. Reduciendo la unidad "sensor reading" de 9bytes a la unidad "Sensong Unit Base" de 7bytes.

<img title="a title" alt="Alt text" src="docs/images/sensing_unit_base.png">

* Codificación envío repetivvio:
* COdificacion envío diferencial:



4. Los datos son envidos mediante transmisión serial RS485  como paquetes de bytes: Como los datos son codificados a bytes estos son enviados de esta manera en paquetes de bytes a través de comunicación serial RS485 estos estan dados por un tamaño máximo de 51 bytes, es decir, 7 Sensing Unit (49 bytes) y 2 bytes para detección de errores. Este tamaño se define debido a que es el tamaño mínimo de un paquete en LoRa (módulo de comunicación inalámbrica utilizada en la superficie del sistema).




Código completo:
El primero es el codigo completo que utiliza la sd para el guardado de los datos, el codigo esta centro

Ejemplos del funcionamiento de medidas:



## Roadmap
backlog
pasos futuros

| N  | Título                                         | Detalle                                                                                                                                                                                                                                                                                                                                                                                                      |
| -- | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 1  | Mover uSD a superficie                         | Hacer que la uSD sea controlada y alimentada desde el nodo de comunicaciones en vez del nodo sensor.                                                                                                                                                                                                                                                                                                         |
| 2  | Bajar ciclos del oscilador a 8Mhz              | Disminuir ciclos del oscilador de 16MHz a 8MHz. Esto disminuye la velocidad a la que opera el sistema sin afectar el funcionamiento de los programas. Funcionando a la mitad de ciclos se espera una disminución del consumo del procesador en el sistema.                                                                                                                                                   |
| 3  | Ocupar VCC 3.3V                                | Bajar alimentación del sistema de 5V a 3.3V. Se debe prototipar y validar el funcionamiento de componentes y electrónica. Con este cambio disminuye el consumo general de energía del sistema obteniendo mayor desempeño de estados de deep-sleep.                                                                                                                                                           |
| 4  | Amplificador universal multiplexado            | Si se caracterizan debidamente las sondas y se verifica que el procesamiento de los circuitos intermediarios se pueden realizar de forma digital, una opción es tener un amplificador para todas las sondas, las cuales mediante multiplexión temporal entren a éste (opcionalmente un digipot puede ayudar a modificar la ganancia). Esto podría reducir drásticamente la cantidad de componentes en placa. |
|    | Integrados de carga de baterías en paralelo    |                                                                                                                                                                                                                                                                                                                                                                                                              |
| 5  | Autonomía de sensores (drift)                  | Si se va a aumentar la autonomía del sistema, es necesario verificar si en ese horizonte de tiempo los sensores serán funcionales.                                                                                                                                                                                                                                                                           |
| 8  | Optimización de componentes                    | Relacionado con los puntos 3 y 4, hay que repasar toda la selección de componentes en términos de funciones y equipos. También se relaciona con la programación.                                                                                                                                                                                                                                             |
|    |                                                |                                                                                                                                                                                                                                                                                                                                                                                                              |
| 6  | Autonomía energética (solar, electrónica)      | Añadir un sistema de carga solar para recargar las baterías 18650 del nodo sensor podría en prácticamente cualquier caso mejorar la autonomía.                                                                                                                                                                                                                                                               |
| 7  | Batería de auto                                | Colocar una batería de auto para todo el sistema teóricamente aseguraría una mayor autonomía.                                                                                                                                                                                                                                                                                                                |
| 9  | Programar nodo a distancia                     | \- Se podría cambiar el atmega por un esp que se programe a distancia.                                                                                                                                                                                                                                                                                                                                       |
| 10 | Switch magnético de encendido                  | Se puede agregar un switching on/off del sistema que se puede activar desde fuera de la carcasa con un imán para de esta manera poder ahorrar energía cuando no se esté utilizando el dispositivo.                                                                                                                                                                                                           |
| 11 | Modularizar datalogger de adaptadores sensores | Separar en placas separadas, pero conectables el datalogger (uC, RTC, SD, energía) de electrónica de amplificación de los sensores dfrobot (pH, turbidez, conductividad)                                                                                                                                                                                                                                     |      z<x
