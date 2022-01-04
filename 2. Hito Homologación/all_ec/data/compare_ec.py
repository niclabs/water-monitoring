#!/usr/bin/python3
import matplotlib.pyplot as plt

# `real_data`: #, ec_solucion (uS)
real_TYPE = [int,            int]
real_data = []
real_header = None
real_SEP = ','
with open('2020-10-22_real_ec.csv', 'r') as f:
    for line in f:
        line = line.strip().replace('\ufeff','')
        if real_header is None:
            real_header = line.split(real_SEP)
            real_data = [ [] for _ in range(len(real_header)) ]
            continue
        if line == '':
            break
        if line[0] == '0': # bad measurement
            continue
        row = line.split(real_SEP)
        for i, (t, d) in enumerate(zip(real_TYPE, row)):
            real_data[i].append( t(d) )

_real_number_col = real_header.index('#')
_real_data_col = real_header.index('ec_solucion')
real_ec = real_data[_real_data_col]

def real_get(n):
    row_index = real_data[_real_number_col].index(n)
    return real_data[_real_data_col][row_index]

SEP = ';'
FILES = ['2020-10-22_data_ECGravity.csv', '2020-10-22_data_TDSGravity1.csv',
         '2020-10-22_data_TDSGravity2.csv',
         '2020-10-22_data_multiparametro.csv']
# `data[i]`: #;   t; <data_sensor>
TYPE = [   int, int,        float]
data = [[] for _ in range (len(FILES))]
header = [None for _ in range(len(FILES))]
for k, filename in enumerate(FILES):
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip().replace('\ufeff','')
            if header[k] is None:
                header[k] = line.split(SEP)
                data[k] = [ [] for _ in range(len(header[k])) ]
                continue
            if line == '':
                break
            row = line.split(SEP)
            for i, (t, d) in enumerate(zip(TYPE, row)):
                data[k][i].append( t(d) )

SENSORS = ['EC.Gravity', 'TDS.Gravity.1', 'TDS.Gravity.2', 'Multiparámetro']

# get: data * label
#   ex: get(data, 'EC.Gravity')
def get(arr, label):
    return arr[ SENSORS.index(label)  ]

def get_time(label):
    this_header = get(header, label)
    time_index = this_header.index('t')
    return get(data, label)[time_index]

def get_values(label):
    this_header = get(header, label)
    value_index = this_header.index(label)
    return get(data, label)[value_index]

def get_indices(label):
    this_header = get(header, label)
    number_index = this_header.index('#')
    return get(data, label)[number_index]

def get_at(label, n):
    this_header = get(header, label)
    number_index = this_header.index('#') # column of counter (probably 0)
    value_index = this_header.index(label) # column of values (probably 2)
    row_index = get(data, label)[number_index].index(n)
    return get(data, label)[value_index][row_index]

def _set(label, new):
    this_header = get(header, label)
    value_index = this_header.index(label)
    data[ SENSORS.index(label) ][value_index] = new

def remove_at(label, n):
    this_header = get(header, label)
    number_index = this_header.index('#')
    row_index = get(data, label)[number_index].index(n)
    sensor_index = SENSORS.index(label)
    for i in range(len(data[sensor_index])):
        data[sensor_index][i].pop(row_index)

# EC.Gravity, TDS.Gravity.1 and TDS.Gravity.2 need preprocessing
IDX_1413_ECG = 11
IDX_1413_TDS1 = 2
IDX_1413_TDS2 = 1
IDX_12880_ECG = 40
IDX_12880_TDS1 = 35
IDX_12880_TDS2 = 30

def slope(v12880, v1413): # slope(x2, x1)
    delta = 12880 - 1413
    return delta/(v12880 - v1413)

def intercept(v12880, v1413):
    return 1413 - slope(v12880, v1413)*v1413

def _voltage_to_ec_ecg(v):
    v2 = get_at('EC.Gravity', IDX_12880_ECG)
    v1 = get_at('EC.Gravity', IDX_1413_ECG)
    #print("EC.Gravity - slope: {sl:.5f}, intercept: {ic:.5f}".format(sl=slope(v2,v1), ic=intercept(v2, v1)))
    return slope(v2,v1)*v + intercept(v2, v1)

def _voltage_to_ec_tds1(v):
    v2 = get_at('TDS.Gravity.1', IDX_12880_TDS1)
    v1 = get_at('TDS.Gravity.1', IDX_1413_TDS1)
    #print("TDS.Gravity.1 - slope: {sl:.5f}, intercept: {ic:.5f}".format(sl=slope(v2,v1), ic=intercept(v2, v1)))
    return slope(v2,v1)*v + intercept(v2, v1)

def _voltage_to_ec_tds2(v):
    v2 = get_at('TDS.Gravity.2', IDX_12880_TDS2)
    v1 = get_at('TDS.Gravity.2', IDX_1413_TDS2)
    #print("TDS.Gravity.2 - slope: {sl:.5f}, intercept: {ic:.5f}".format(sl=slope(v2,v1), ic=intercept(v2, v1)))
    return slope(v2,v1)*v + intercept(v2, v1)

## DFRobot_EC uses two different K, one for high EC (12880 uS) and one for low
## EC (1413 uS)
## ref: https://github.com/DFRobot/DFRobot_EC/blob/06eb47856ef9b579fd1a80258b8d77b178fec666/DFRobot_EC.cpp#L62
threshold_ec = 5000
## The threshold choice of 5000 is an arbitrary midpoint. Further research is advised.

def voltage_to_ec_ecg(v):
    first_ec = _voltage_to_ec_ecg(v)
    if first_ec < threshold_ec: # low
        K = 1413/get_at('EC.Gravity', IDX_1413_ECG)
        #print("ECG K_low = %.5f" % K)
    else: # high
        K = 12880/get_at('EC.Gravity', IDX_12880_ECG)
        #print("ECG K_high = %.5f" % K)
    return K*v

def voltage_to_ec_tds1(v):
    # Alternate calibration from https://github.com/DFRobot/GravityTDS/blob/e711cd1ea5f410b257cf8a59cd548d7c8b131428/GravityTDS.cpp#L177
    #v_1413 = get_at('TDS.Gravity.1', IDX_1413_TDS1)
    #K = 1413/(133.42*v_1413**3 - 255.86*v_1413**2 + 857.39*v_1413)
    #return K*(133.42*v**3 - 255.86*v**2 + 857.39*v)
    first_ec = _voltage_to_ec_tds1(v)
    if first_ec < threshold_ec: # low
        K = 1413/get_at('TDS.Gravity.1', IDX_1413_TDS1)
        #print("TDS.1 K_low = %.5f" % K)
    else: # high
        K = 12880/get_at('TDS.Gravity.1', IDX_12880_TDS1)
        #print("TDS.1 K_high = %.5f" % K)
    return K*v

def voltage_to_ec_tds2(v):
    # Alternate calibration from GravityTDS' GitHub repo
    #v_1413 = get_at('TDS.Gravity.2', IDX_1413_TDS2)
    #K = 1413/(133.42*v_1413**3 - 255.86*v_1413**2 + 857.39*v_1413)
    #return K*(133.42*v**3 - 255.86*v**2 + 857.39*v)
    first_ec = _voltage_to_ec_tds2(v)
    if first_ec < threshold_ec: # low
        K = 1413/get_at('TDS.Gravity.2', IDX_1413_TDS2)
        #print("TDS.2 K_low = %.5f" % K)
    else: # high
        K = 12880/get_at('TDS.Gravity.2', IDX_12880_TDS2)
        #print("TDS.2 K_high = %.5f" % K)
    return K*v

"""
# This assumed an affine linear behaviour between 1413 and 12880
_set('EC.Gravity', list(map(_voltage_to_ec_ecg, get_values('EC.Gravity'))))
_set('TDS.Gravity.1', list(map(_voltage_to_ec_tds1, get_values('TDS.Gravity.1'))))
_set('TDS.Gravity.2', list(map(_voltage_to_ec_tds2, get_values('TDS.Gravity.2'))))
"""
## overwrite
# This assumes a linear behaviour locally around 1413, and locally around 12880
# (each with it's own slope)
_set('EC.Gravity', list(map(voltage_to_ec_ecg, get_values('EC.Gravity'))))
_set('TDS.Gravity.1', list(map(voltage_to_ec_tds1, get_values('TDS.Gravity.1'))))
_set('TDS.Gravity.2', list(map(voltage_to_ec_tds2, get_values('TDS.Gravity.2'))))

## Now, considering the calibration points within the dataset would be cheating
## (for error calculations)
remove_at('EC.Gravity', IDX_12880_ECG) # We're using `pop` (remove higher index first)
remove_at('EC.Gravity', IDX_1413_ECG)
remove_at('TDS.Gravity.1', IDX_12880_TDS1)
remove_at('TDS.Gravity.1', IDX_1413_TDS1)
remove_at('TDS.Gravity.2', IDX_12880_TDS2)
remove_at('TDS.Gravity.2', IDX_1413_TDS2)

# Ready to plot

TO_PLOT = SENSORS
COLORS = ['green', 'red', 'orange', '#17becf']

N = len(data[0])

# First plot: EC vs EC
for color, label in zip(COLORS, TO_PLOT):
    indices = get(data, label)[0] # first column should be measurement index
    ys = get_values(label)
    xs = list(map(real_get, indices))
    plt.scatter(xs, ys, c=color, s=20, label=label)

plt.plot([min(real_data[_real_data_col]), max(real_data[_real_data_col])],
         [min(real_data[_real_data_col]), max(real_data[_real_data_col])],
         c='b', label='real')
plt.xlabel('Real EC (calibration solution) [$\mu S/cm$]')
plt.ylabel('Measured EC [$\mu S/cm$]')
plt.title('EC vs EC')
plt.legend()
plt.show()

import numpy as np
# Second plot: EC over time
time_all = {}
values_all = {}
for color, label in zip(COLORS, TO_PLOT):
    t = list(map(lambda t: t/(1000*60), get_time(label)))
    ys = get_values(label)
    time_all[label] = t
    values_all[label] = ys
    plt.scatter(t, ys, c=color, label=label)

## Real time
T_1 = 59010 # ms
T_44 = 2740533 # ms (at index: 44)
real_time = np.linspace(T_1, T_44, 44)
real_time_mins = list(map(lambda t: t/(1000*60), real_time))
plt.plot(real_time_mins, real_ec, c='b', label='real')
plt.xlabel('Time [min]')
plt.ylabel('EC [$\mu S$]')
plt.title('EC over time')
plt.legend()
plt.show()

# Third plot: Square error over time
for color, label in zip(COLORS, TO_PLOT):
    ys = np.array(get_values(label))
    indices = get_indices(label)
    real_ys = np.array(list(map(real_get, indices)))
    t = list(map(lambda t: t/(1000*60), get_time(label)))
    abs_errors = abs(real_ys - ys)
    plt.scatter(t, abs_errors, c=color, label=label)

plt.xlabel('Time [min]')
plt.ylabel('Absolute error $\mid EC_{real} - EC_{meas} \mid$')
plt.title('Absolute error')
plt.legend()
plt.show()

# Fourth plot: Square error over time
square_errors_all = {}
for color, label in zip(COLORS, TO_PLOT):
    ys = np.array(get_values(label))
    indices = get_indices(label)
    real_ys = np.array(list(map(real_get, indices)))
    square_errors = (real_ys - ys)**2
    square_errors_all[label] = square_errors
    plt.scatter(time_all[label], square_errors, c=color, label=label)

plt.xlabel('Time [min]')
plt.ylabel('Square error $(EC_{real} - EC_{meas})^2$')
plt.title('Square error')
plt.legend()
plt.show()

print('Root-mean-square error (uS):')
for color, label in zip(COLORS, TO_PLOT):
    ys = np.array(get_values(label))
    N = len(ys)
    indices = get_indices(label)
    real_ys = np.array(list(map(real_get, indices)))
    square_errors = (real_ys - ys)**2
    square_errors = square_errors[ np.where( np.logical_not( np.isnan(square_errors)) )]
    rmse = np.sqrt( sum(square_errors) / len(square_errors) )
    print('{sensor} ({n}):\t{error:.3f}'.format(sensor=label, n=N, error=rmse))

# Fifth plot: EC over time and SqErr over time comparison
fig, axs = plt.subplots(2,1)
## (2nd plot)
for color, label in zip(COLORS, TO_PLOT):
    axs[0].scatter(time_all[label], values_all[label], c=color, label=label)

axs[0].plot(real_time_mins, real_ec, c='b', label='real')
axs[0].set_ylabel('EC [$\mu S$]')
axs[0].set_title('EC over time')
## (4th plot)
for color, label in zip(COLORS, TO_PLOT):
    axs[1].scatter(time_all[label], square_errors_all[label], c=color, label=label)
axs[1].set_xlabel('Time [min]')
axs[1].set_ylabel('Square error $(EC_{real} - EC_{meas})^2$')
axs[1].set_title('Square error')
axs[1].legend()
fig.tight_layout()
plt.show()
