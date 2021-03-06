# %%
"""
Tutorial: Send Data to Firebase Using Raspberry Pi
Hardware:
– Raspberry Pi 3 Model B
– ADS1x15 ADC with analogic sensors
- DS18b20 sensors

References:
– https://circuitpython.readthedocs.io/projects/mlx90614/en/latest/
- https://tutorial.cytron.io/2020/12/09/send-data-to-firebase-using-raspberry-pi/
– https://github.com/thisbejim/Pyrebase
"""
 
import time
import board
import busio as io
import pyrebase


import os
import glob
 
# Import the ADS1x15 module.
import Adafruit_ADS1x15

os.system("modprobe w1-gpio")
os.system("modprobe w1-therm")


# Create an ADS1115 ADC (16-bit) instance.
#adc = Adafruit_ADS1x15.ADS1015()

# Or create an ADS1015 ADC (12-bit) instance.
adc = Adafruit_ADS1x15.ADS1015()

# Note you can change the I2C address from its default (0x48), and/or the I2C
# bus by passing in these optional parameters:
#adc = Adafruit_ADS1x15.ADS1015(address=0x49, busnum=1)

# Choose a gain of 1 for reading voltages from 0 to 4.09V.
# Or pick a different gain to change the range of voltages that are read:
#  - 2/3 = +/-6.144V
#  -   1 = +/-4.096V
#  -   2 = +/-2.048V
#  -   4 = +/-1.024V
#  -   8 = +/-0.512V
#  -  16 = +/-0.256V
# See table 3 in the ADS1015/ADS1115 datasheet for more info on gain.
GAIN = 1

#%% Configuración de la Base de datos:

config = {
 "apiKey": "Q1Efd77KWUK60suxePFpIPo9tZvoqidnI2rl4SmL",
 "authDomain": "water-monitoring-46b05.firebaseapp.com",
 "databaseURL": "https://water-monitoring-46b05-default-rtdb.firebaseio.com/",
 "storageBucket": "project-id.appspot.com"
}
 
# Se inicializa la conexión a firebase
firebase = pyrebase.initialize_app(config)
db = firebase.database()
 
print('Reading ADS1x15 values, DS18B20 temperature values,') 
print(" Data to Firebase Using Raspberry Pi")
print('press Ctrl-C to quit...')

 
cabeceras = ['timestamp', 'ch_1', 'ch_2','ch_3','ch_4','temp_1','temp_2','temp_3']
# Print nice channel column headers.
print('| {0:>18} | {1:>6} | {2:>6} | {3:>6} | {4:>6} | {5:>6} | {6:>6} | {7:>6} |'.format(*cabeceras))
print('-' * 87)


#%% Loop:
# Main loop.
while True:
    t0 = time.time()
    # Read all the ADC channel values in a list.
    values = [0]*5
    for i in range(4):
        # Read the specified ADC channel using the previously set gain value.
        values[i+1] = adc.read_adc(i, gain=GAIN)
        # Note you can also pass in an optional data_rate parameter that controls
        # the ADC conversion time (in samples/second). Each chip has a different
        # set of allowed data rate values, see datasheet Table 9 config register
        # DR bit values.
        #values[i] = adc.read_adc(i, gain=GAIN, data_rate=128)
        # Each value will be a 12 or 16 bit signed integer value depending on the
        # ADC (ADS1015 = 12-bit, ADS1115 = 16-bit).
    
    values[0] = t0
    raw1str = "{:.2f}".format(values[1])
    raw2str = "{:.2f}".format(values[2])
    raw3str = "{:.2f}".format(values[3])
    raw4str = "{:.2f}".format(values[4])
    
    raw1 = float(raw1str)
    raw2 = float(raw2str)
    raw3 = float(raw3str)
    raw4 = float(raw4str)    

    
    Directorios = glob.glob( "/sys/bus/w1/devices/28*/"+"*temperature")
    n_sensores = len(Directorios)
    temperatura=[-1,-1,-1]
    if n_sensores >= 1:
        for i in range(n_sensores):
            fSensor = open(Directorios[i])

            linSensor = fSensor.readlines()
    
            strTemp = linSensor[0][:5]
            temperatura[i] = float(strTemp) / 1000.0

            # print("The temperature %d is %s celsius" % (i, temperatura[i]))

        Directorios = glob.glob( "/sys/bus/w1/devices/28*/"+"*temperature")
        n_sensores = len(Directorios) 
    
    data = {
    "timestamp": values[0],
    "channel 1": raw1,
    "channel 2": raw2,
    "channel 3": raw3,
    "channel 4": raw4,
    "ds18b20_1": temperatura[0],
    "ds18b20_2": temperatura[1],
    "ds18b20_3": temperatura[2],
    }
    values+=temperatura
    print('| {0:>18} | {1:>6} | {2:>6} | {3:>6} | {4:>6} | {5:>6} | {6:>6} | {7:>6} |'.format(*values))
    tf=time.time()

    while tf-t0 < 30:
        time.sleep(0.01)
        tf=time.time()
    # print('| {0:>6} | {1:>6} | {2:>6} | {3:>6} | {4:>12} |'.format(*values))
    # print('{0:>6} | {1:>6} | {2:>6} |'.format(*temperatura))

    # Se envían los datos a la realtime database de firebase.
    try:
        db.child("datalogger").child("1-set").set(data)
        db.child("datalogger").child("2-push").push(data)   
        last_sent = time.time()

    # Si no es posible subir los datos, printea el mensaje de error en el log.
    except:
        print('Ocurrió un error al subir la data a FireBase. Por favor chequear la conexión a internet del dispositivo y las reglas de escritura de la Real Time DataBase')
        print('El último dato se envió en', last_sent)

    # Comprobación tipo watchdog timer, si no ha enviado datos a la nube en 15 minutos, se reinicia.
    if time.time()-last_sent > 900:
        print('Watchog timer reiniciará el dispositivo.')
        time.sleep(1)
        os.system('sudo reboot')

 
# %%

