# 📦 Carcasa sumergible

Revisaremos el diseño y construcción de una carcasa incorporada al nodo sensor con factibilidad de ser desplegada en aguas superficiales y subterráneas. Se han realizado pruebas en ambiente controlado (laboratorio) sumergiendo la carcasa en tanques de agua por diferentes períodos de tiempo y evaluando efectividad en la protección del interior de acuerdo al grado de protección IP alcanzado (norma internacional CEI 60529 Degrees of Protection).

## Primer prototipo

El primer prototipo se fabrico para realizar pruebas con los sensores de presión (nivel de agua) que se tenían que sumergir en un pozo unos ~5 metros. Se usaron materiales de plomería genéricos y se rellenó con arroz para absorver la humedad en caso de filtraciones.

<img title="a title" alt="Alt text" src="images/carcasa_primera_version_2.png" width="200px">
<img title="a title" alt="Alt text" src="images/carcasa_primera_version.jpeg" width="300px">

Este prototipo estuvo sumergido unos 45 minutos y al sacaro si tenía bastante humedad dentro, no tenía un "pozo de agua" pero si mucha humedad que en el tiempo dañaría la electrónica. Se cree que las filtraciones fueron por las tapas de los sensores de presión.

<!-->
## Directrices de diseño (”requerimientos”)
- Condiciones de uso
<-->

<!--
## Metodología: Diseño Orientado a la Manufactura (DfM)
-->

<!-- (TODO: revisar bien.) -->

<!-->
Luego del primer prototipo se trabajó junto a un equipo de diseñadores basados en la metodología Design for Manufacturing o Diseño Orientado a la Manufactura.

Es una metodología de diseño que plantea principios fundamentales que guían las decisiones de diseño desde etapas tempranas del proceso creativo, con el objetivo de generar objetos o sistemas pensados para su fabricación, reduciendo así número de piezas, tiempos, costos, y recursos involucrados, para generar productos que respondan a los recursos disponibles en el proyecto de forma pertinente. En el Diseño Orientado a la Manufactura, se priorizan procesos y materiales disponibles de acuerdo a la escala de producción requerida, en el caso del nodo al ser un proyecto en etapa temprana, donde se prototipará a baja escala, se priorizará procesos locales y del menor costo posible. Además, la geometría de cada una de las piezas deberá ser diseñada para un proceso de fabricación en específico, priorizando piezas simétricas, además de utilizar piezas y componentes ya disponibles en el mercado para disminuir el número de piezas especiales a fabricar. Finalmente se deben escoger materiales que no solo cumplan con los requerimientos técnicos necesarios sino también que sean de bajo costo, y maquinables mediante los procesos de manufactura seleccionados. --> 

## Segundo prototipo

Una vez modelados los sensores y componentes, se comenzó a diseñar a nivel general el dispositivo, considerando los requerimientos establecidos previamente y priorizando materiales accesibles y disponibles en el mercado local. La geometría inicial contempla un tubo de acrílico transparente, tapas y un soporte interno para la electrónica, además de espacios laterales para la salida de los sensores y cable UTP, como se muestra en la Figura 11. Las tapas laterales (Figura 12) realizan el sello total del dispositivo en esta versión, mediante o`rings y fijándose al soporte interno que es lo que mantendrá las tapas a presión dentro del sistema.

IP69.  Además se incorporó al sistema un enganche (Figura 15) de acero inoxidable desde donde se anclará un mosquetón al momento del descenso, tanto en aguas superficiales como en aguas subterráneas.

Finalmente, se diseñó un sistema protector contra impactos al momento del descenso, en acero inoxidable.


<img title="a title" alt="Alt text" src="images/carcasa_prototipo_0.jpg" width="300px">


El sistema se prototipo y se realizaron diferentes pruebas a  los componentes de sello de los sensores y el equipos completo en un tubo de pvc de 6 metros.

<img title="a title" alt="Alt text" src="images/carcasa_prototipo_1.jpg" width="300px">

Se fueron ajustando partes hasta que no hubo filtraciones. :)
<img title="a title" alt="Alt text" src="images/carcasa_prototipo_2.jpg" width="300px">

Se probó este ptototipo luego en un pozo en el sector de Laguna Caren, aquí se realizaron pruebas sumergiendolo a mayores profundidades progresivamente (10, 20, 30 y 40mt).

<img title="a title" alt="Alt text" src="images/carcasa_validacion_1.jpg" width="300px">

Hubo de todos los resultados, positivos donde el permanecio sumergido sin filtraciones y también con filtraciones. Se observan algunos casos de carcasa con filtraciones más abajo.

<img title="a title" alt="Alt text" src="images/carcasa_validacion_2.jpg" width="300px">

<img title="a title" alt="Alt text" src="images/carcasa_validacion_3.jpg" width="300px">

### Archivos de diseño y fabricación

Se pueden encontrar los archivos de diseño, materiales e instrucciones en el siguiente link.

https://github.com/niclabs/water-monitoring/tree/master/5.%20Hito%20Carcasa

<img title="a title" alt="Alt text" src="images/Nodo completo.JPG">

## Siguientes pasos

Se puede ordenar el trabajo futuro en dos categorías dependiendo de su objetivo:

##### To Do's:

1. Disminución de costo de materiales.
2. Estandarización de proceso de fabricación.

##### To Fix:

1. Revisar mecanismo de sellado y filtraciones.



<!--

- mayores dificultades, tipos de pruebas, desafíos.

Diseño final y partes


## Instrucciones armado

Manual (Adjunto)

## Resultados pruebas

Método de las pruebas

pruebas en tubo rancagua

pruebas en terreno

pruebas antes de llevar a terreno

Resumen de resultados y roadmap siguiente.

Comparación de costos cable vs cable nosotros.

- Tips
- Consejos
- Desafíos

Aprendizajes.

-->
