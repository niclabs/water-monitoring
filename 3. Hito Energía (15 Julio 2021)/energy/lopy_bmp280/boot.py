import machine
from bmp280 import *
import time

i2c = machine.I2C()
bmp = BMP280(i2c)
# https://github.com/dafvid/micropython-bmp280
#bmp.use_case(BMP280_CASE_WEATHER)
bmp.use_case(BMP280_CASE_HANDHELD_LOW)
