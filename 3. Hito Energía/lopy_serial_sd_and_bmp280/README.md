# lopy_serial_sd_and_bmp280

Conexión de la LoPy con un Arduino mediante UART. Cada vez que la LoPy recibe un mensaje del Arduino, lo almacena utilizando el lector de microSD de la tarjeta de expansión.

- Código LoPy (`LoPy4_sd`) basado en `LoPy4_serial`
- Código Arduino (`dummy_data_sender`) basado en `arduino_nodo_sensor`

Cableado:
- Conectar, a través de un adaptador de nivel, los pines de comunicación serial de la LoPy y del Arduino según dice `LoPy4_serial/boot.py` y `dummy_data_sender/dummy_data_sender.ino`, respectivamente. *CAVEAT*: Usar pines P3 y P6. El P4 lo utiliza la LoPy para comunicarse con la tarjeta SD.
- Conectar la LoPy a la tarjeta de expansión
- El contenido del buffer del script de Arduino debería ser `0xF0000000000015F100000009001FF200000013000EF30000001D0010F4000000270006` (35 bytes). Esto también se incluye en el archivo `dummy.bin`.
- El mensaje recibido vía UART por la LoPy, una vez descartado el primer byte, debería ser `b'f0000000000015f100000009001ff200000013000ef30000001d0010f4000000270006'`. Esto calza con el mensaje enviado.
- Los datos decodificados son `[(0, 0, 2.1), (1, 9, 3.1), (2, 19, 1.4), (3, 29, 1.6), (4, 39, 0.6)]`, consistentes con lo codificado desde el Arduino.
