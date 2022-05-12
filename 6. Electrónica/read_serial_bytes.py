import serial
import binascii as binascii

# this port address is for the serial tx/rx pins on the GPIO header
SERIAL_PORT = '/dev/cu.usbserial-00000000'
# be sure to set this to the same rate 
# used on the Arduino
SERIAL_RATE = 9600

def main():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    while True:
        # using ser.readline() assumes each line contains a single reading
        # sent using Serial.println() on the Arduino
        reading = ser.read()#line()#.decode('utf-8')
        # reading is a string...do whatever you want from here
        print(reading  + ", " + binascii.hexlify(reading))


if __name__ == "__main__":
    main()