# 💻 Electrónica

## Diseño
Selección de componentes
Prototipo
Sensores


<img title="a title" alt="Alt text" src="images/Diagrama Alimentación.png">


## Historial de Versiones
bugs
mejoras
mejoras no realizadas

<img title="a title" alt="Alt text" src="images/PCBnombrada_v0.png">

## Fabricación
cotización
fabricación
armado

## Programación
versiones
codificación


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
| 11 | Modularizar datalogger de adaptadores sensores | Separar en placas separadas, pero conectables el datalogger (uC, RTC, SD, energía) de electrónica de amplificación de los sensores dfrobot (pH, turbidez, conductividad)                                                                                                                                                                                                                                     |
