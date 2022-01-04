# Librerias para la lectura de la temperatura
import os
import glob
import time

os.system("modprobe w1-gpio")
os.system("modprobe w1-therm")

Directorios = glob.glob( "/sys/bus/w1/devices/28*/"+"*temperature")
n_sensores = len(Directorios)
temperatura=[-1,-1,-1]
while n_sensores >= 1:
    for i in range(n_sensores):
        fSensor = open(Directorios[i])

        linSensor = fSensor.readlines()
  
        strTemp = linSensor[0][:5]
        temperatura[i] = float(strTemp) / 1000.0

        print("The temperature %d is %s celsius" % (i, temperatura[i]))

    Directorios = glob.glob( "/sys/bus/w1/devices/28*/"+"*temperature")
    n_sensores = len(Directorios) 
    # print(n_sensores) 
    temperatura=[-1,-1,-1]
    # time.sleep(1)

