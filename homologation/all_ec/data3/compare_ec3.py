#!/usr/bin/python3
import matplotlib.pyplot as plt
import numpy as np

"""
N;MultiP;TDS.1;MultiP;TDS.2;T1;T2;Sol;Comm
1;1555;1.284.180;1587;1.210.937;27.2;27.0;1413;medida de calibración
"""
SEP = ';'
# `data`:    N, MultiP.1, TDS.1, MultiP.2, TDS.2,    T1,    T2,   Sol, Comm
TYPE = [   int,    float, float,    float, float, float, float, float,  str]
data = {}
header = None
with open('2020-11-02_tds.csv', 'r') as f:
    for line in f:
        line = line.strip().replace('\ufeff','')
        if header is None:
            header = line.split(SEP)
            data = dict([(x, []) for x in header])
            continue
        if line == '':
            break
        row = line.split(SEP)
        for i, (t, d) in enumerate(zip(TYPE, row)):
            try:
                val = t(d)
            except ValueError:
                float('nan')
            data[header[i]].append( val )

#           TDS.Gravity.1, TDS.Gravity.2
SENSORS = ['TDS.1',        'TDS.2']

# Returns the label of the Multiparameter column corresponding to
# a given sensor column
def label_multi(label):
    return header[header.index(label)-1]

# https://github.com/DFRobot/GravityTDS/blob/e711cd1ea5f410b257cf8a59cd548d7c8b131428/GravityTDS.cpp#L177
def calibrated_K(v, EC=1413):
    return EC/(133.42*v**3 - 255.86*v**2 + 857*v)

# Plot each point
plt.scatter(data['N'], data['TDS.1'], c='red', label='TDS.1')
plt.scatter(data['N'], data['TDS.2'], c='orange', label='TDS.2')
plt.ylabel('Voltage [mV]')
plt.xlabel('#')
plt.title('Datapoints (voltage)')
plt.show()



# Plot Reference EC vs TDS voltage
plt.plot(data['TDS.1'], data[label_multi('TDS.1')], 'o--', c='red',
         label='TDS.1')
plt.plot(data['TDS.2'], data[label_multi('TDS.2')], 'o--', c='orange',
         label='TDS.2')
for sensor in ['TDS.1', 'TDS.2']:
    for x,y,n in zip(data[sensor], data[label_multi(sensor)], data['N']):
        plt.annotate("[{}]{}".format(sensor,n), (x,y), fontsize=6)
plt.xlabel('Voltage [mV]')
plt.ylabel('Reference EC [$\mu$S/cm]')
plt.title('Reference (multiparameter) for each sensor')
plt.legend()
plt.show()


import scipy.optimize
# Fit to EC = ln(a*Voltage + b)
def tds_fit_function(v, a, b, c):
    return a*v**b + c*v #a*np.log(v + b)
fit_tds1, _ = scipy.optimize.curve_fit(tds_fit_function,
                                       data['TDS.1'],
                                       data[label_multi('TDS.1')],
                                      maxfev=20000)
print("TDS1: {}".format(fit_tds1))
fit_tds2, _ = scipy.optimize.curve_fit(tds_fit_function,
                                       data['TDS.2'],
                                       data[label_multi('TDS.2')],
                                      maxfev=20000)
print("TDS2: {}".format(fit_tds2))

"""
# a_TDS1, b_TDS1, c_TDS1, d_TDS1 = np.polyfit(data['TDS.1'], data[label_multi('TDS.1')],
fit_tds1 = np.polyfit(data['TDS.1'], data[label_multi('TDS.1')],
                                    3)
print("TDS1: a=%.5f\tb=%.5f\tc=%.5f\td=%.5f" % (a_TDS1, b_TDS1, c_TDS1, d_TDS1))
fit_tds2 = np.polyfit(data[label_multi('TDS.2')], data['TDS.2'],
                                    3)
print("TDS2: a=%.5f\tb=%.5f\tc=%.5f\td=%.5f" % (a_TDS2, b_TDS2, c_TDS2, d_TDS2))
"""
"""
plt.scatter(data[label_multi('TDS.1')], data['TDS.1'], c='red', label='TDS.1')
plt.scatter(data[label_multi('TDS.2')], data['TDS.2'], c='orange', label='TDS.2')
for sensor in ['TDS.1', 'TDS.2']:
    for x,y,n in zip(data[label_multi(sensor)], data[sensor], data['N']):
        plt.annotate("[{}]{}".format(sensor,n), (x,y), fontsize=6)
"""
plt.plot(data[label_multi('TDS.1')][:-1], list(map(lambda v :
                                                   tds_fit_function(v, *fit_tds1),
                                           data['TDS.1'][:-1])), '*-', c='red', label='TDS.1 Fit')
plt.plot(data[label_multi('TDS.2')][:-1], list(map(lambda v :
                                                   tds_fit_function(v, *fit_tds2),
                                           data['TDS.2'][:-1])), '*-', c='orange', label='TDS.2 Fit')
multi_labels = list(map(label_multi, ['TDS.1', 'TDS.2']))
multi_data = list(map(lambda label: data[label], multi_labels))
min_multi = min(map(min, multi_data))
max_multi = max(map(max, multi_data))
plt.plot([min_multi, max_multi],
         [min_multi, max_multi],
         c='b', label='real')

plt.xlabel('Reference EC (multiparameter) [$\mu S/cm$]')
plt.ylabel('Measured EC [$\mu S/cm$]')
plt.title('TDS EC(voltage) fit: $a v^{b} + c v$')
plt.legend()
plt.show()


# TDS.Gravity.1 and TDS.Gravity.2 need preprocessing
"""
K_TDS1_1413 = 0.73822
K_TDS1_12880 = 5.30749
K_TDS2_1413 = 1.03721
K_TDS2_12880 = 6.51315
"""

THRESHOLD = 2.5

"""
def voltage_to_ec_tds1(v):
    ec = K_TDS1_1413*v
    if ec > THRESHOLD:
        ec = K_TDS1_12880*v
    return ec

def voltage_to_ec_tds2(v):
    ec = K_TDS2_1413*v
    if ec > THRESHOLD:
        ec = K_TDS2_12880*v
    return ec
"""


# First data point is calibration point (1413 uS/cm)
K_TDS1 = calibrated_K(data['TDS.1'][0])
K_TDS2 = calibrated_K(data['TDS.2'][0])

print(f'K (TDS.1): {K_TDS1}')
print(f'K (TDS.2): {K_TDS2}')

def voltage_to_ec_tds1(v):
    return (133.42*v**3 - 255.86*v**2 + 857.39*v)*K_TDS1

def voltage_to_ec_tds2(v):
    return (133.42*v**3 - 255.86*v**2 + 857.39*v)*K_TDS2


## overwrite
data['TDS.1'] = list(map(voltage_to_ec_tds1, data['TDS.1']))
data['TDS.2'] = list(map(voltage_to_ec_tds2, data['TDS.2']))

# Ready to plot

TO_PLOT = SENSORS
COLORS = ['green', 'red', 'orange']

N = max(data['N'])

# First plot: EC vs EC
for color, label in zip(COLORS, TO_PLOT):
    ys = data[label]
    xs = data[label_multi(label)]
    plt.scatter(xs, ys, c=color, s=20, label=label)

multi_labels = list(map(label_multi, TO_PLOT))
multi_data = list(map(lambda label: data[label], multi_labels))
min_multi = min(map(min, multi_data))
max_multi = max(map(max, multi_data))
plt.plot([min_multi, max_multi],
         [min_multi, max_multi],
         c='b', label='real')
plt.xlabel('Reference EC [$\mu S/cm$]')
plt.ylabel('Measured EC [$\mu S/cm$]')
plt.title('EC vs EC (using GravityTDS\' cubic formula)')
plt.legend()
plt.show()

print('No time. Exiting.')
exit(1)

#sensors_data = list(map(lambda label: data[label], TO_PLOT))
# Second plot: EC over time
for color, label in zip(COLORS, TO_PLOT):
    ys = data[label]
    real_ys = data[label_multi(label)]
    plt.scatter(get_time(), ys, c=color, label=label)
    plt.plot(get_time()[:-1], real_ys[:-1], '--', c=color, label=label_multi(label))

## Real time
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
