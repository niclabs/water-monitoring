# Documentación Arduino Nodo Sensor V1

El siguiente documento tiene por fin explicar el módulo arduino_nodo_sensor.ino, código desarrollado para el módulo Arduino del Nodo 
Sensor. El orden de la información detallada en este documento es congruente con la estructura del código implementado.

## Información general

El código implementado puede dividirse en 4 secciones principales, las cuales respetan el siguiente orden de aparición:

1. Importación de librerías
2. Definición y declaración de variables globales
3. Definición de funciones
4. Funciones setup() y loop() del entorno Arduino.

En las secciones 2 y 3 se utilizan comentarios separadores de la forma "// ------ Nombre Sección --------", con el objetivo de separar el código implementado y detallar su uso. Así, hablaremos de la sección del código Nombre Sección, cuando queramos referirnos al código ubicado desde el separador "// ------ Nombre Sección ------" hasta la aparición del siguiente separador.

## Importación de librerías

En esta sección de código se importan las distintas librerías necesarias para la gestión de los distintos dispositivos conectados al Arduino Nano. Dentro de éstas, destaca la librería "SensorPayload.h", la cual implementa las codificaciones utilizadas para la reducción de consumo energético en el proceso de transmisión inalámbrica.

## Definición y declaración de variables globales

A continuación se detallan las distintas secciones presentes en el código, con explicaciones breves de su uso y finalidad.

### Debug option

Esta sección define una macro DEBUG, la cual se utiliza para condicionar el uso de prints Seriales en el Arduino. Cuando la macro DEBUG toma el valor 1, se definen los métodos debugPrint y debugPrintln, los cuales implementan los métodos Serial.print y Serial.println de Arduino, respectivamente. Cuando la macro DEBUG toma el valor 0, los metodos debugPrint y debugPrintln se definen como vacíos, omitiéndose los prints del programa y ahorrando el espacio en memoria de los strings a imprimir.

Por último, se define la macro ENCODE_MODE, la cual establece el tipo de codificacion que se usará en el proceso de codificación de las lecturas de los sensores. Las opciones disponibles para el valor de esta macro son BASE_MODE, DIF_MODE y REP_MODE, para el caso de las codificaciones base, diferencial y repetición, respectivamente.

### Software Serial port

El Arduino Nano cuenta únicamente con un puerto para comunicación serial, el cual se usa al conectar el dispositivo mediante USB. Por esta razón, se utiliza un puerto Serial por Software en los pines digitales 8 y 9, con el cual se realizan los envíos de información desde el Arduino Nano hacia la LoPy4.

### DS18B20

Se define la macro ONE_WIRE_BUS, la cual establece el pin digital en que estará conectada la salida de datos del sensor  DS18B20. Luego, se definen dos objetos utilizados para la gestión con el sensor.

### SensorPayload

Se define una macro que establece la cantidad máxima de lecturas de 7 bytes que se codificarán antes de ser enviadas. El valor que se utiliza en la implementación es 7, pues al codificar 7 lecturas de 7 bytes cada una se asegura el envío de todos los datos codificados en un paquete LoRa de tamaño mínimo (51 bytes).

### RTC

En esta sección se definen macros y variables asociadas al funcionamiento del módulo RTC y su respectiva rutina de interrupciones.

La macro CLOCK_INTERRUPT_PIN establece el pin digital donde se recibirá la señal de interrupción gatillada por el módulo RTC.

Las macros SENSE_FLAG y SEND_FLAG establecen valores arbitrarios utilizados para diferenciar los eventos de Senseo de Datos y Envío de Datos, gatillados por la rutina de interrupción del RTC.

El objeto rtc se usa para gestionar el módulo RTC a nivel de software, utilizando la librería RTClib.

La variable alarm_time se utiliza para almacenar la hora en la que se ejecutará la siguiente interrupción gatillada por el módulo RTC.

La variable elapsed_time se utiliza para medir el tiempo transcurrido desde el último evento de envío de datos. Se utiliza para configurar las alarmas venideras dentro del sistema.

La variable volatil alarm_flag almacena el evento que debe ejecutarse posterior a una interrupción de reloj.

### LoPy timer

La variable startTime se utiliza para medir el tiempo transcurrido desde que se dio aviso al módulo LoPy4 para comenzar el envío de datos.

### File configuration variables

En esta sección se establecen macros y variables asociadas a la gestion de los archivos a generar dentro de la tarjeta SD.

La macro FILE_BASE_NAME establece el nombre de los archivos a generar, mientras que TMP_FILE_NAME establece el nombre del archivo de trabajo en que se almacenarán temporalmente los datos.

La variable FILE_BLOCK_COUNT establece el número de bloques de 512 bytes que almacenará un archivo.

La variable binName almacena el nombre del último archivo generado en la tarjeta SD. Respeta el patrón "dataXXX.bin", donde XXX es el número de archivo generado. Se inicializa con el valor data000.bin.

El arreglo first_pending_file almacena el primer archivo a ser enviado cuando el evento de envío de datos sea ejecutado. La generación de archivos es secuencial sobre el identificador XXX, por lo que si en este arreglo se almacenan los valores 005, se enviarán los datos desde el archivo data005.bin hasta el último archivo con mayor numeración que esté presente en la tarjeta SD. 

La variable pending_blocks almacena la cantidad de bloques pendientes por enviar en el archivo de trabajo data###.bin. Esta variable se utiliza para establecer que el primer dato a ser enviado está presente en el archivo de trabajo.

La variable next_to_send almacena la posición dentro del buffer de trabajo del siguiente elemento que debe ser enviado.

### SD configuration variables

En esta sección se definen las macros y variables asociadas a la gestión de la tarjeta SD.

La macro CHIP_SELECT almacena el pin CS del Arduino.

La macro error(msg) define un método para la impresión de un error asociado a la tarjeta SD, deteniendo la ejecución del programa.

El objeto sd se utilza para gestionar la tarjeta SD en el sistema.

El objeto binFile se utiliza para el manejo de un archivo dentro de la tarjeta SD. Dado que el archivo binFile es una única instancia de un SdBaseFile, el programa está pensado para trabajar con un único archivo abierto a la vez.

La variable sd_block es una estructura de datos de tipo block_type. Esta estuctura de datos es la que se escribe en los archivos binarios usados en la gestión de la tarjeta SD. Para detalles de la estructura, ver el archivo dataTypes.h

La variable written_blocks es un contador utilizado para llevar la cuenta de los bloques escritos en el archivo de trabajo utilizado.

### Sensors configuration variables

En esta sección se definen macros asociadas a la configuración tanto de muestreo como de envío de datos. Además, se define un typedef para estandarizar las funciones asociadas a la lectura de datos.

La macro SENSING_FREQ_SECS se utiliza para definir el tiempo que debe transcurrir entre cada muestreo de los sensores.

La macro SENDING_FREQ_SECS establece el tiempo que debe transcurirr entre cada envío de datos hacia el módulo de transmisión LoPy4

La macro N_SENSORS se usa para definir la cantidad de sensores que se están utilizando en el Nodo Sensor.

El typedef sensorValueFunction define un tipo que representa una función que no recibe argumentos y retorna un float.

El arreglo registered_sensors se utiliza para identificar los distintos sensores que se encuentran dentro del Nodo Sensor. La identificación de estos sensores se genera mediante los identificadores únicos asociados a cada uno de los sensores, definidos dentro del archivo dataTypes.h

Posterior al arreglo registered_sensors, se deben definir las funciones asociadas a la lectura de cada uno de los sensores registrados en arreglo.

Finalmente, se define el arreglo de sensorValueFunction sensors_functions, el cual debe almacenar las funciones asociadas a cada uno de los sensores registrados en registered_sensors, cuidando respetar el orden de registro de cada uno de los sensores. Ejemplo de esto se encuentra comentado en el código.

## Definición de funciones

En esta sección, se definen funciones asociadas a distintas tareas dentro del Nodo Sensor. Las funciones están documentadas en el código, especificando la tarea que realizan. Cabe destacar que, debido a la arquitectura del programa desarrollado en el Arduino, las funciones trabajan con las variables globales definidas en la sección anterior, por lo que generalmente no reciben parámetros y modifican estos valores a lo largo de la ejecución.



