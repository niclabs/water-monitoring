'''
Simple demo
BMP280 pressure and temperature sensor
'''
print('[main.py reached]')

i = 1
while True:
    print('Loop', i)
    i+=1
    temp = bmp.temperature
    pres = bmp.pressure
    print('Measured temp:', temp, '- pressure:', pres)
    print('Going to sleep')
    machine.sleep(5*1000)