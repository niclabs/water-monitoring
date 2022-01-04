import struct

def decode_reading_type(byte_array, start=0):
    if len(byte_array[start:]) < 9:
        raise ValueError('Need 9 bytes to decode')
    value_part = byte_array[start:start+4]
    ts_part = byte_array[start+4:start+8]

    value = struct.unpack('<f', value_part)[0]
    ts = struct.unpack('<I', ts_part)[0]
    sensor = byte_array[start+8]
    return (value, ts, sensor)

def decode_bin(byte_array):
    n = len(byte_array)
    arr = []
    i = 0
    while i < n:
        try:
            data = decode_reading_type(byte_array, i)
        except ValueError as e:
            print('Could not decode rest:', byte_array[i])
            return arr
        arr.append(data)
        i += 9
    return arr


def main(*args):
    if len(args) < 2:
        print('Uso %s [nombre archivo a decodificar]' % args[0])
        exit(1)
    with open(args[1], 'rb') as f:
        print(decode_bin(f.read()))

if __name__ == '__main__':
    import sys
    main(*sys.argv)
