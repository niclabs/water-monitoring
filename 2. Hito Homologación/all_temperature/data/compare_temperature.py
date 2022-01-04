#!/usr/bin/python3
import matplotlib.pyplot as plt

with open('2020-10-07_all_temperature.txt', 'r') as f:
    real_temp = f.read().split('\n')
    if real_temp[0].startswith('Multi'):
        real_temp = real_temp[1:]
    if real_temp[-1] == '':
        real_temp = real_temp[:-1]
    real_temp = list(map(float, real_temp))

# `real_temp`: °C read from multiparameter

# `data`: t  DS18B20-1 ..                  ..5  thermistor Atlas
TYPE = [int, float, float, float, float, float, int,       float]
data = []
header = None
SEP = '; '
with open('2020-10-07_data_sensores_homologacion.csv', 'r') as f:
    for line in f:
        line = line.strip()
        if header is None:
            header = line.split(SEP)
            data = [ [] for _ in range(len(header)) ]
            continue
        if line == '':
            break
        row = line.split(SEP)
        for i, (t, d) in enumerate(zip(TYPE, row)):
            data[i].append( t(d) )

def get(data, col_name):
    return data[ header.index(col_name) ]

'''
vin = map(lambda x: 5*x/1024, data)

M_THERMISTOR = -573.7977  # °C/Ohm
N_THERMISTOR = 26051.8387 # °C
VCC_THERMISTOR = 5        # V
RA_THERMISTOR = 9890     # Ohm

thermistor_temp = list(map(lambda v:
                           (1/M_THERMISTOR)*(RA_THERMISTOR*(VCC_THERMISTOR-v)/v
                                             - N_THERMISTOR), vin))
'''

TO_PLOT = ['DS18B20.1', 'DS18B20.2', 'DS18B20.3', 'DS18B20.4',
           'DS18B20.5', 'Atlas']
COLORS = ['#9c0000', '#ff6969', '#ff7300', '#ffa800', '#b88f00', '#0e7700']

N = len(data[0])

# First plot: Temperature vs Temperature
for i in range(N):
    for color, label in zip(COLORS, TO_PLOT):
        plt.scatter(real_temp[i], get(data, label)[i], c=color, s=20)

plt.plot([min(real_temp), max(real_temp)], [min(real_temp), max(real_temp)],
         c='b')
plt.xlabel('Real temperature [°C]')
plt.ylabel('Measured Temperature [°C]')
plt.title('red tones: DS18B20\'s; green: Atlas')
plt.show()

# Second plot: Temperature over time
t = list(map(lambda t: t/(1000*60), get(data, 't')))
for color, label in zip(COLORS, TO_PLOT):
    plt.plot(t, get(data, label), c=color)
plt.plot(t, real_temp, c='b')
plt.xlabel('Time [min]')
plt.ylabel('Temperature [°C]')
plt.title('red tones: DS18B20\'s; green: Atlas; blue: multiparameter')
plt.show()

import numpy as np
# Third plot: Square error over time
real_temp_np = np.array(real_temp)
data_np = list(map(lambda ls: np.array(ls), data))
square_errors = [(data_i - real_temp_np)**2 for data_i in data_np]
for color, label in zip(COLORS, TO_PLOT):
    plt.plot(t, get(square_errors, label), c=color)
plt.xlabel('Time [min]')
plt.ylabel('Square error $(T_{real} - T_{meas})^2$')
plt.title('red tones: DS18B20\'s; green: Atlas')
plt.show()

print('Root-mean-square error (°C):')
for label in TO_PLOT:
    rmse = np.sqrt( sum(get(square_errors, label)) / N )
    print('{sensor}:\t{error:.3f}'.format(sensor=label, error=rmse))
