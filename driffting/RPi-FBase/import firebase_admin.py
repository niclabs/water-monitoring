#%% Credenciales y conexión
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import time


# Fetch the service account key JSON file contents
cred = credentials.Certificate('/home/pi/Documents/water-monitoring/driffting/RPi-FBase/firebase-adminsdk.json')
# Initialize the app with a service account, granting admin privileges
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://water-monitoring-46b05-default-rtdb.firebaseio.com/'
})

#%% Referencia
ref = db.reference('datalogger/2-push')
# db = ref.get()
# print(ref.limit_to_last(5).get())

# %% Prueba
# snapshot = ref.order_by_child('timestamp').get()
# keys=list(snapshot)
#
# for key, val in snapshot.items():

#     print('key: {0} values{1}'.format(key, val{''}))
# for key, dict in snapshot.items():
#     print('KEY: {0} VALUE: {1}'.format(key, dict[key]))    
# snapshot[keys[0]][temperature[]]

# %% 

for key, val in db.items():
    # print(type(key))
    # print(val)
    try:
        ts = time.ctime(val['timestamp'])
        
        val['timestamp'] = ts
        print(val['timestamp'])
    except:
        val['timestamp'] = float("Nan")
        
    print(db[key]['timestamp'])
    # print(db[key]['channel 1'])

# %%  Almacenamiento de variables
db = ref.get()
t = []
an1 = []
an2 = []
an3 = []
an4 = []
temp1 = []
temp2 = []
temp3 = []
for key, val in db.items():
    # print(type(key))
    # print(val)
    try:
        
        ts = time.ctime(val['timestamp'])
        t += [ts]
        an1 += [val['channel 1']]
        an2 += [val['channel 2']]
        an3 += [val['channel 3']]
        an4 += [val['channel 4']]
        temp1 += [val['ds18b20_1']]
        temp2 += [val['ds18b20_2']]
        temp3 += [val['ds18b20_3']]
        # val['timestamp'] = ts
        # print(val['timestamp'])
    except:
        val['timestamp'] = float("NaN")
        t +=[float("NaN")]
        an1 += [float("NaN")]
        an2 += [float("NaN")]
        an3 += [float("NaN")]
        an4 += [float("NaN")]
        temp1 += [float("NaN")]
        temp2 += [float("NaN")]
        temp3 += [float("NaN")]
        
        
# %%
# PLots
import matplotlib.pyplot as plt
plt.plot(t,temp1)
# %%
print(len(t))
# %%
t