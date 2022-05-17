import os
import machine
import pycom
import socket
import ubinascii
import struct
import uctypes
from network import LoRa
from network import Sigfox
from machine import UART
import time
import math
from machine import Pin
from lib.error_check import check_fletcher16


print('========== Starting LoRaWAN configuration ==========')
lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.AU915, adr=False)
print(ubinascii.hexlify(lora.mac()).upper())
# create an ABP authentication params
dev_addr = struct.unpack(">l", ubinascii.unhexlify('2603193F'))[0]
nwk_swkey = ubinascii.unhexlify('A48C359E506AE2AC7C0D747AC39F2869')
app_swkey = ubinascii.unhexlify('FD97194F98F1BE97E56E19CBDD8B4396')
# create an OTAA authentication params
# app_eui = ubinascii.unhexlify('0000000000000000')
# app_key = ubinascii.unhexlify('7557FCC7BB76CAC19E35D858FD1E60E0')

# Remove unused channels
for i in range(0,8):
    lora.remove_channel(i)
for i in range(16,65):
    lora.remove_channel(i)
for i in range(66,72):
    lora.remove_channel(i)

# join a network using ABP (Activation By Personalization)
lora.join(activation=LoRa.ABP, auth=(dev_addr, nwk_swkey, app_swkey))
#lora.join(activation=LoRa.OTAA, auth=(app_eui, app_key), timeout=0)

# wait until the module has joined the network
while not lora.has_joined():
    time.sleep(2.5)
    print('Not yet joined...')
print("LoPy4 - Has joined!!!!!")

# create a LoRa socket
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)

# set the LoRaWAN socket options
DATA_RATE = 0
s.setsockopt(socket.SOL_LORA, socket.SO_DR, DATA_RATE)
s.setsockopt(socket.SOL_LORA, socket.SO_CONFIRMED, True)

print('========== Starting Serial UART configuration ===========')
# Configure second UART bus on pins P3(TX1) and P4(RX1)
uart = machine.UART(1, baudrate=19200, bits=8, parity=None, stop=1, pins=('P3', 'P4'))

print('========== Starting PIN wakeup configuration ============')
#p_in = Pin('P10', mode=Pin.OUT, pull=None, alt=-1)
machine.pin_sleep_wakeup(['P4'], machine.WAKEUP_ALL_LOW, False)
#machine.enable_irq()

pycom.heartbeat(False)

time.sleep(5)

machine.main('main.py')
print('========== Starting main.py ==========')
