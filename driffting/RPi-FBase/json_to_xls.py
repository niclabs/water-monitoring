import pandas as pd
df_json = pd.read_json('/home/pi/Documents/water-monitoring-46b05-default-rtdb-2-push-export.json', lines=True)
# df_json.to_excel('DATAFILE_PUSH.xlsx')

#%%
df_json.head()
