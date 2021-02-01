"""
Tutorial: Send Data to Firebase Using Raspberry Pi
Hardware:
– Raspberry Pi 3 Model B
– ADS1x15
References:
– https://circuitpython.readthedocs.io/projects/mlx90614/en/latest/
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

#%%%%%% dejar tutorial
config = {
 "apiKey": "Q1Efd77KWUK60suxePFpIPo9tZvoqidnI2rl4SmL",
 "authDomain": "water-monitoring-46b05.firebaseapp.com",
 "databaseURL": "https://water-monitoring-46b05-default-rtdb.firebaseio.com/",
 "storageBucket": "project-id.appspot.com"
}
 
firebase = pyrebase.initialize_app(config)
db = firebase.database()
 
print('Reading ADS1x15 values, press Ctrl-C to quit...') 
print("And send Data to Firebase Using Raspberry Pi")
print("—————————————-")
print()
 

# Print nice channel column headers.
print('| {0:>6} | {1:>6} | {2:>6} | {3:>6} | {4:>18} |'.format(*range(5)))
print('-' * 60)
# Main loop.
while True:
    # Read all the ADC channel values in a list.
    values = [0]*5
    for i in range(4):
        # Read the specified ADC channel using the previously set gain value.
        values[i] = adc.read_adc(i, gain=GAIN)
        # Note you can also pass in an optional data_rate parameter that controls
        # the ADC conversion time (in samples/second). Each chip has a different
        # set of allowed data rate values, see datasheet Table 9 config register
        # DR bit values.
        #values[i] = adc.read_adc(i, gain=GAIN, data_rate=128)
        # Each value will be a 12 or 16 bit signed integer value depending on the
        # ADC (ADS1015 = 12-bit, ADS1115 = 16-bit).
    
    values[4] = time.time()
    raw1str = "{:.2f}".format(values[0])
    raw2str = "{:.2f}".format(values[1])
    raw3str = "{:.2f}".format(values[2])
    raw4str = "{:.2f}".format(values[3])
    
    raw1 = float(raw1str)
    raw2 = float(raw2str)
    raw3 = float(raw3str)
    raw4 = float(raw4str)
    
    # print("log 1: {} ".format(raw1str))
    
    # print("log 2: {} ".format(raw2str))
    # print("log 3: {} ".format(raw3str))
    # print("log 4: {} ".format(raw4str))
    # print()
    
    
    time.sleep(2)
    # Print the ADC values.
    print('| {0:>6} | {1:>6} | {2:>6} | {3:>6} | {4:>12} |'.format(*values))
    # Pause for half a second.

    
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
    "timestamp": values[4],
    "channel 1": raw1,
    "channel 2": raw2,
    "channel 3": raw3,
    "channel 4": raw4,
    "ds18b20_1": temperatura[0],
    "ds18b20_2": temperatura[1],
    "ds18b20_3": temperatura[2],
    }
    print('| {0:>6} | {1:>6} | {2:>6} | {3:>6} | {4:>12} |'.format(*values))
    print('{0:>6} | {1:>6} | {2:>6} |'.format(*temperatura))
    db.child("datalogger").child("1-set").set(data)
    db.child("datalogger").child("2-push").push(data)   
    time.sleep(1)
