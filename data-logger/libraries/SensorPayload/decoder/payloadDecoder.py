import struct as st
import traceback 
import base64

class BadEncodingException(Exception):
    pass

def base_decoding(readings:list, byte_array:bytes, pointer:int):
    """
    Base decoding of data packets.
    It appends the values decoded to the given readings inside a tuple with the format (sensor_id, timestamp, value).
    Input: readings: list where readings are being stored
           byte_array: bytes containing sensor's readings
           pointer: position in the bytearray where decoding will be made
    Output: int: number of bytes decoded
    """
    sensor_id = byte_array[pointer] & 15 # 15 is 00001111
    pointer += 1
    timestamp = st.unpack('>I', byte_array[pointer: pointer+4])[0]
    pointer += 4
    sign = -1 if (byte_array[pointer] & 128) else 1
    raw_value = (2**15 - 1) & st.unpack('>H', byte_array[pointer:pointer+2])[0]
    value = sign * (raw_value/10)
    readings.append((sensor_id, timestamp, value))
    return 7

def repeated_decoding(readings:list, byte_array:bytes, pointer:int):
    """
    """
    # Base SensingUnit
    ref_sensor_id = byte_array[pointer] & 15 # 15 is 00001111
    pointer += 1
    ref_timestamp = st.unpack('>I', byte_array[pointer: pointer+4])[0]
    pointer += 4
    sign = -1 if (byte_array[pointer] & 128) else 1
    raw_value = (2**15 - 1) & st.unpack('>H', byte_array[pointer:pointer+2])[0]
    ref_value = sign * (raw_value/10)
    pointer += 2
    # Repeated SensingUnit
    if(byte_array[pointer] & 192 != 128): # 128 = 10000000
        raise BadEncodingException("Adjacent SensingUnit does not have 10 as Coding Info")
    sensor_id = byte_array[pointer] & 15 # 15 is 00001111
    if(sensor_id != ref_sensor_id):
        raise BadEncodingException("Adjacent SensingUnit does not have same Sensor ID")
    pointer += 1
    time_interval = st.unpack('>H', byte_array[pointer: pointer+2])[0]
    pointer += 2
    number_of_values = st.unpack('>I', byte_array[pointer: pointer+4])[0]
    readings.append((ref_sensor_id, ref_timestamp, ref_value))
    for i in range(1, number_of_values+1):
        readings.append((ref_sensor_id, ref_timestamp+i*time_interval, ref_value))
    return 14


def diferential_decoding(readings:list, byte_array:bytes, pointer:int):
    """
    """
     # Base SensingUnit
    ref_sensor_id = byte_array[pointer] & 15 # 15 is 00001111
    pointer += 1
    ref_timestamp = st.unpack('>I', byte_array[pointer: pointer+4])[0]
    pointer += 4
    sign = -1 if (byte_array[pointer] & 128) else 1
    raw_value = (2**15 - 1) & st.unpack('>H', byte_array[pointer:pointer+2])[0]
    ref_value = sign * (raw_value/10)
    pointer += 2
    total_size = 7
    readings.append((ref_sensor_id, ref_timestamp, ref_value))
    # Diferential SensingUnit
    while(pointer < len(byte_array) and byte_array[pointer] & 192 == 64): # 64 = 01000000
        sensor_id = byte_array[pointer] & 15 # 15 is 00001111
        if(sensor_id != ref_sensor_id):
            return total_size
        pointer += 1
        dif_timestamp1 = st.unpack('>H', byte_array[pointer: pointer+2])[0]
        pointer += 2
        dif_value1_sign = -1 if (byte_array[pointer] & 128) else 1
        raw_dif_value1 = (2**7-1) & st.unpack('>B', byte_array[pointer: pointer+1])[0]
        dif_value1 = dif_value1_sign * (raw_dif_value1/10)
        pointer +=1
        dif_timestamp2 = st.unpack('>H', byte_array[pointer: pointer+2])[0]
        pointer += 2
        dif_value2_sign = -1 if (byte_array[pointer] & 128) else 1
        raw_dif_value2 = (2**7-1) & st.unpack('>B', byte_array[pointer: pointer+1])[0]
        dif_value2 = dif_value2_sign * (raw_dif_value2/10)
        pointer +=1
        readings.append((ref_sensor_id, ref_timestamp+dif_timestamp1, ref_value+dif_value1))
        readings.append((ref_sensor_id, ref_timestamp+dif_timestamp2, ref_value+dif_value2))
        total_size += 7
    print(total_size)
    return total_size


def end_decoding(readings:list, byte_array:bytes, pointer:int):
    """
    Function called if the coding info is 00, which means that there is no more data to decode.
    Input: readings: list where readings are being stored
           byte_array: bytes containing sensor's readings
           pointer: position in the bytearray where coding info appeared
    Output: int: size of byte array
    """
    return len(byte_array)


def decode_payload(payload:str):
    """
    Decodes a given bytearray into a set of measures with timestamp.
    Input: payload: base64 string representing the bytes containing sensor's readings
    Output: [ (int, int, float) ]: list of tuples containing sensor ID, timestamp and sensed value
    """
    byte_array = base64.b64decode(payload.encode('ascii'))
    readings = []
    array_size = len(byte_array)
    pointer = 0
    decoders = {
                192: base_decoding,
                128: repeated_decoding,
                64: diferential_decoding,
                0: end_decoding
               }
    while(pointer < array_size):
        try:
            coding_info = 192 & byte_array[pointer]
            pointer += decoders[coding_info](readings, byte_array, pointer)
        except:
            print(f"Error: wrong coding information on sensing unit. Byte position inside bytearray: {pointer}")
            traceback.print_exc() 
            return None
    return readings


