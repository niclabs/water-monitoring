#!/usr/bin/python3
import matplotlib.pyplot as plt
import numpy as np

#N,C.E.,MultiP.0,T.0,TDS.1,MultiP.1,T.1,TDS.2,MultiP.2,T.2,time,arduino N,comm,.
#1,0,4.58,20.6,0,7.36,20,0,3.46,20.1,7:20 AM,14-Jan,,
#2,92.773,639,20.2,297.852,184.4,20.2,214.844,254,20.3,7:34,15-23,"Se agrega súp

SEP = ','
# `data`:    N,   C.E, MultiP.0,   T.0, TDS.1, MultiP.1,   T.1, TDS.2, MultiP.2,   T.2, time, arduino N, comm
TYPE = [   int, float,    float, float, float,    float, float, float,    float, float,  str,       str, str]
data = {}
header = None
with open('2020-10-30_datos_ce.csv', 'r') as f:
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

#           EC.Gravity, TDS.Gravity.1, TDS.Gravity.2
SENSORS = ['C.E.',             'TDS.1',        'TDS.2']

def get_time():
    time_column = data['time']
    def hh_mm_to_min(hh_mm):
        hh, mm = hh_mm.split(':')
        return 60*int(hh) + int(mm) - 440 # 440 = '7:20'
    return list(map(hh_mm_to_min, time_column))

# Returns the label of the Multiparameter column corresponding to
# a given sensor column
def label_multi(label):
    return header[header.index(label)+1]

"""
plt.scatter(data['N'], data['TDS.1'], c='red', label='TDS.1')
plt.scatter(data['N'], data['TDS.2'], c='orange', label='TDS.2')
plt.ylabel('Voltage [mV]')
plt.xlabel('#')
plt.show()
"""

# Remove values according to criterion
def remove_values(filter_func):
    to_remove = set()
    for reference_column in [data[label_multi(label)] for label in SENSORS]:
        for row_index, value in enumerate(reference_column):
            if filter_func(value):
                to_remove.add(row_index)
    for index in sorted(to_remove, reverse=True): # Must be done in reverse order (we're removing data from a list)
        for key in data:
            data[key].pop(index)


# Fitting TDS.Gravity values
for k in data:
    # N=15 looks like an outlier
    data[k].pop(14)
    # Last datum is out of range
    data[k].pop()

# Remove values over 3500
remove_values(lambda ec: ec > 3500)


# Plot Reference EC vs TDS voltage
plt.plot(data['TDS.1'], data[label_multi('TDS.1')], 'o--', c='red',
         label='TDS.1')
plt.plot(data['TDS.2'], data[label_multi('TDS.2')], 'o--', c='orange',
         label='TDS.2')
"""
# Tag number of each measure
for sensor in ['TDS.1', 'TDS.2']:
    for x,y,n in zip(data[sensor], data[label_multi(sensor)], data['N']):
        plt.annotate("[{}]{}".format(sensor,n), (x,y), fontsize=6)
"""
plt.xlabel('Voltage [mV]')
plt.ylabel('Reference EC [$\mu$S/cm]')
plt.title('TDS characteristic')
plt.legend()
plt.show()


import scipy.optimize
# Fit to EC = ln(a*Voltage + b)
def tds_fit_function(v, a, b, c):
    return a*v**2 + b*v + c#a*v**b + c*v #a*np.log(v + b)
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
plt.legend()
plt.suptitle('TDS EC(voltage) fit: $a v^2 + b v + c$', y=1.005) # $a v^{b} + c v$', y=1.005)
plt.title(('TDS.1: a={:.3e}, b={:.3e}, c={:.3e}\n'
           'TDS.2: a={:.3e}, b={:.3e}, c={:.3e}').format(fit_tds1[0],fit_tds1[1],fit_tds1[2],
                                                         fit_tds2[0],fit_tds2[1],fit_tds2[2]),
         fontsize=10)

plt.show()

# EC.Gravity, TDS.Gravity.1 and TDS.Gravity.2 need preprocessing
K_ECG_1413 = 5.67417
K_ECG_12880 = 5.86183
K_TDS1_1413 = 0.73822
K_TDS1_12880 = 5.30749
K_TDS2_1413 = 1.03721
K_TDS2_12880 = 6.51315

THRESHOLD = 2.5

def voltage_to_ec_ecg(v):
    ec = K_ECG_1413*v
    if ec > THRESHOLD:
        ec = K_ECG_12880*v
    return ec

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

def calibrated_K(v, EC=1413):
    return EC/(133.42*v**3 - 255.86*v**2 + 857*v)

# only for gravity TDS
def calibrate_label(label, k):
    # Using k-th data point
    return calibrated_K(data[label][k], EC=data[label_multi(label)][k])

original_data = data.copy()
for calibration in ["USING_FIT", "USING_GRAVITY_FORMULA"]:
    print(f"---{calibration}---")
    if calibration == "USING_FIT":
        def voltage_to_ec_tds1(v):
            return tds_fit_function(v, *fit_tds1)
        def voltage_to_ec_tds2(v):
            return tds_fit_function(v, *fit_tds2)
    elif calibration == "USING_GRAVITY_FORMULA":
        K_TDS1 = calibrate_label('TDS.1', 1)
        K_TDS2 = calibrate_label('TDS.2', 1)
        print(f'K (TDS.1): {K_TDS1}')
        print(f'K (TDS.2): {K_TDS2}')
        def voltage_to_ec_tds1(v):
            return (133.42*v**3 - 255.86*v**2 + 857.39*v)*K_TDS1
        def voltage_to_ec_tds2(v):
            return (133.42*v**3 - 255.86*v**2 + 857.39*v)*K_TDS2

    ## overwrite
    # On C.E, this assumes a linear behaviour locally around 1413, and locally around 12880
    # (each with it's own slope)
    data['C.E.'] = list(map(voltage_to_ec_ecg, original_data['C.E.']))
    data['TDS.1'] = list(map(voltage_to_ec_tds1, original_data['TDS.1']))
    data['TDS.2'] = list(map(voltage_to_ec_tds2, original_data['TDS.2']))

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
    plt.xlabel('Real EC (calibration solution) [$\mu S/cm$]')
    plt.ylabel('Measured EC [$\mu S/cm$]')
    plt.title(f'EC vs EC [{calibration}]')
    plt.legend()
    plt.show()


    #sensors_data = list(map(lambda label: data[label], TO_PLOT))
    # Second plot: EC over time
    for color, label in zip(COLORS, TO_PLOT):
        ys = data[label]
        real_ys = data[label_multi(label)]
        plt.scatter(get_time(), ys, c=color, label=label)
        plt.plot(get_time(), real_ys, '--', c=color, label=label_multi(label))

    ## Real time
    plt.xlabel('Time [min]')
    plt.ylabel('EC [$\mu S$]')
    plt.title('EC over time')
    plt.legend()
    plt.show()

    # Third plot: Square error over time
    for color, label in zip(COLORS, TO_PLOT):
        ys = np.array(data[label])
        indices = data['N']
        real_ys = np.array(data[label_multi(label)])
        abs_errors = abs(real_ys - ys)
        plt.scatter(get_time(), abs_errors, c=color, label=label)

    plt.xlabel('Time [min]')
    plt.ylabel('Absolute error $\mid EC_{real} - EC_{meas} \mid$')
    plt.title(f'Absolute error {calibration}')
    plt.legend()
    plt.show()

    # Fourth plot: Square error over time
    square_errors_all = {}
    for color, label in zip(COLORS, TO_PLOT):
        ys = np.array(data[label])
        real_ys = np.array(data[label_multi(label)])
        square_errors = (real_ys - ys)**2
        square_errors_all[label] = square_errors
        plt.scatter(get_time(), square_errors, c=color, label=label)

    plt.xlabel('Time [min]')
    plt.ylabel('Square error $(EC_{real} - EC_{meas})^2$')
    plt.title(f'Square error {calibration}')
    plt.legend()
    plt.show()

    print('Root-mean-square error (uS):')
    for color, label in zip(COLORS, TO_PLOT):
        ys = np.array(data[label])
        N = len(ys)
        real_ys = np.array(data[label_multi(label)])
        square_errors = (real_ys - ys)**2
        square_errors = square_errors[ np.where( np.logical_not( np.isnan(square_errors)) )]
        rmse = np.sqrt( sum(square_errors) / len(square_errors) )
        print('{sensor} ({n}):\t{error:.3f}'.format(sensor=label, n=N, error=rmse))

    # Fifth plot: EC over time and SqErr over time comparison
    fig, axs = plt.subplots(2,1)
    ## (2nd plot)
    for color, label in zip(COLORS, TO_PLOT):
        axs[0].scatter(get_time(), data[label], c=color, label=label)
        axs[0].plot(get_time(), data[label_multi(label)], '--', c=color, label='ref. '+label)
    axs[0].set_ylabel('EC [$\mu S$]')
    axs[0].set_title('EC over time')
    ## (4th plot)
    for color, label in zip(COLORS, TO_PLOT):
        axs[1].scatter(get_time(), square_errors_all[label], c=color, label=label)
    axs[1].set_xlabel('Time [min]')
    axs[1].set_ylabel('Square error $(EC_{real} - EC_{meas})^2$')
    axs[1].set_title(f'Square error {calibration}')
    axs[1].legend()
    fig.tight_layout()
    plt.show()
