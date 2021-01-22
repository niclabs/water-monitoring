import time                             #Importamos el paquete time
from w1thermsensor import W1ThermSensor #Importamos el paquete W1ThermSensor

sensor = W1ThermSensor() #Creamos el objeto sensor

while True:
    temperature = sensor.get_temperature()                #Obtenemos la temperatura en centígrados
    print("The temperature is %s celsius" % temperature)  #Imprimimos el resultado
    time.sleep(1)                                         #Esperamos un segundo antes de terminar el ciclo 
    