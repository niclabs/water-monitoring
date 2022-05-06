# Sensores 🌡 

<!--
TODO: poner modelo de sensores de referencia.
TODO: poner cosas que salieron mal: medicion presión a 40cm, ...)
TODO: poner fotos de todos los experimentos para todos.
TODO: terminar de analizar íultimos datos.
TODO: poner tabla de resuemn resultados sensores.
-->

<img title="a title" alt="Alt text" src="images/sensor_agua.png">

Los sensores otorgarán el input de información y de ellos dependerá la calidad de los datos en el sistema. Por esto son de vital importancia dado que mejores datos se espera lleven a mejores estimaciones y alertas. Pero mejores sensores implican un mayor costo, y se espera generar un sistema de monitoreo acequible.

El objetivo entonces es encontrar sensores económico cuya calidad de medición sea la suficiente para operar el sistema, entendiendo que los sensores profesionales por lo general tienen un desempeño y una presición mayor de la necesaria para el análisis de agua.

## Opciones
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

## Estado de resultados

| Variable                   | Aprobado | Estado | Comentarios |
| -------------------------- | -------- | ------ | ----------- |
| Temperatura | ✅ Buenos resultados |        |             |
| Presión     | ✅ Buenos resultados |        |             |
| pH          | ✅ Buenos resultados |        |             |
| Conductividad Eléctrica | ⚠️ Se realizarán más experimentos |        |             |
| Turbiedad   | ❌ No aprobado, se buscarán otros modelos |        |             |

### Temperatura

<img title="a title" alt="Alt text" src="images/sensor_temp.png" width="200px">
<img title="a title" alt="Alt text" src="images/sensor_temp_experimento1.png" width="300px">

Experimentos demostraron un buen desempeño del sensor en su comportamiento en el tiempo. Algunas observaciones:

- Comportamiento muy predecible, después de un tiempo se puede observar una muy leve desviación lineal constante de la medidas, pero se puede corregir completamente reajustando un offset.
- Errores menores a 0.5ºC luego de 2 meses  de uso prolongado sin recalibrar. Luego de 9 meses de uso error en mediciones aumento pero se mantiene menor a los 0.5ºC al recalibrar. 

<img title="a title" alt="Alt text" src="images/sensor_temp_plot_hist.png">

### Conductividad

<img title="a title" alt="Alt text" src="images/sensor_tds.png" width="300px">

Algunas observaciones:

- Al medir dos al mismo tiempo se afectan la medida.
- Es súper clara la medición dentro del rango de 0 - 1500 uS/cm.
    - Al medir hasta 1500 uS/cm errores se mantienen bajo los 50 uS/cm. Al medir hasta 2000 uS/cm error medio se mantiene entre 100 - 200 uS/cm.

- Luego de 3 meses de uso continuo, sensor pierde su calibración Se esperan más experimentos para ver si es por depositos de minerales en el sensor, o si es corregible, o fue un error puntual, o etc.
- No se ve histéresis importante.

### pH

<img title="a title" alt="Alt text" src="images/sensor_ph.jpeg" width="300px">

- No se ve mucha histéresis. Hay harta variación en la calibración pero al recalibrar los sensores vuelven a funcionar.
- Se esperar hacer más experimento para corroborar datos y además poder ir estimando que tan rápido se descalibran los datos, y cuánto se va perdiendo de presición con cada recalibración.

### Nivel de Agua

<img title="a title" alt="Alt text" src="images/sensor_presion.png" width="200px">
<img title="a title" alt="Alt text" src="images/sensor_presion_atm.png" width="200px">

- Para medir el nivel de una columna de agua se usan dos sensores: uno en el fondo del pozo y otro en la superficie para corregir las variaciones en la presión atmosférica que también siente el sensor sumergido. 
- Experimentos fueron positivos, el error de medición fue menor a 1 cm en diferentes experimentos y luego de 3 meses de medición continua equipo sigue midiendo correctamente, no se observa desviación en su medición.

<img title="a title" alt="Alt text" src="images/sensor_presion_experimento1_1.jpeg" width="300px">
<img title="a title" alt="Alt text" src="images/sensor_presion_experimento1_2.jpeg" width="300px">
<img title="a title" alt="Alt text" src="images/sensor_presion_experimento1_0.png" width="300px">

### Turbidez

<img title="a title" alt="Alt text" src="images/sensor_turbidez.png" width="200px">

- Sensores son muy sensibles a la luz, entonces mediciones varían ampliamente dependiendo de la luz ambiente y la posición en que se coloquen, cualquier movivmiento o cambio de alguna de estas condiciones afectará la medición.
- Errores del rango de ~1000 NTU. No confiables, quizás solo apra alertas grandes pero mejor seguir buscando.
## Aprendizajes
- tener un buen setup del experimento
