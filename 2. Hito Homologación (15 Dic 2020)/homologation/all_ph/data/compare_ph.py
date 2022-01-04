#!/usr/bin/python3
import matplotlib.pyplot as plt

# `data`: t  DS18B20-1 ..                  ..5  thermistor Atlas
# `data`: abs#; Hora;   t; pHGravity.1; pHGravity.2; Atlas; Multiparámetro; Comment
TYPE = [   int,float, int,       float,       float, float,          float, str]
data = []
header = None
SEP = ';'
with open('2020-10-14_data_ph.csv', 'r') as f:
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
            if d == '': # If no data in this cell
                data[i].append(data[i][-1]) # repeat last value
            else:
                data[i].append( t(d) )

def get(data, col_name):
    return data[ header.index(col_name) ]

def _set(data, col_name, new):
    data[ header.index(col_name) ] = new

real_ph = get(data, 'Multiparámetro')

# pHGravity.1 and pHGravity.2 need preproccesing
# real_ph[2] is taken at pH 4
# real_ph[4] is taken at pH 7
IDX_PH_4 = 2
IDX_PH_7 = 4
for label in ['pHGravity.1', 'pHGravity.2']:
    data_gravity = get(data, label)
    voltage_ph_4 = data_gravity[IDX_PH_4]
    voltage_ph_7 = data_gravity[IDX_PH_7]
    # https://github.com/DFRobot/DFRobot_PH/blob/20f3a836998fbc40ad39568b60b17aff1d55c0c4/DFRobot_PH.cpp#L61
    slope = (7 - 4) / ((voltage_ph_7-1500)/3 - (voltage_ph_4-1500)/3)
    intercept = 7 - slope*(voltage_ph_7-1500)/3
    def voltage_to_ph(v):
        return slope*(v-1500)/3 + intercept
    _set(data, label, list(map(voltage_to_ph, data_gravity))) # overwrite

TO_PLOT = ['pHGravity.1', 'pHGravity.2', 'Atlas']
COLORS = ['red', 'green', 'orange']

N = len(data[0])

# First plot: pH vs pH
for i in range(N):
    for color, label in zip(COLORS, TO_PLOT):
        plt.scatter(real_ph[i], get(data, label)[i], c=color, s=20)

plt.plot([min(real_ph), max(real_ph)], [min(real_ph), max(real_ph)],
         c='b')
plt.xlabel('Real pH')
plt.ylabel('Measured pH')
plt.title('red: gravity1; green: gravity2; orange: atlas')
plt.show()

# Second plot: pH over time
t = list(map(lambda t: t/(1000*60), get(data, 't')))
for color, label in zip(COLORS, TO_PLOT):
    plt.plot(t, get(data, label), c=color)
plt.plot(t, real_ph, c='b')
plt.xlabel('Time [min]')
plt.ylabel('pH')
plt.title('red: gravity1; green: gravity2; orange: atlas')
plt.show()

import numpy as np
# Third plot: Square error over time
real_ph_np = np.array(real_ph)
data_np = list(map(lambda ls: np.array(ls), data))
square_errors = [(data_i - real_ph_np)**2 if type(data_i[0]) != np.str_ else [] for data_i in data_np]

TO_PLOT.remove('Atlas') # Too high
for color, label in zip(COLORS, TO_PLOT):
    plt.plot(t, get(square_errors, label), c=color)
plt.xlabel('Time [min]')
plt.ylabel('Square error $(pH_{real} - pH_{meas})^2$')
plt.title('red: gravity1; green: gravity2; atlas too big for scale')
plt.show()

print('Root-mean-square error (pH):')
for label in TO_PLOT:
    rmse = np.sqrt( sum(get(square_errors, label)) / N )
    print('{sensor}:\t{error:.3f}'.format(sensor=label, error=rmse))
