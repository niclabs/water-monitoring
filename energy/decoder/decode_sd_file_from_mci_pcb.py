#/opt/anaconda3/bin/python

import payloadDecoder as pd
import unittest
import base64
import binascii

def bitstring_to_bytes(s):
    return int(s, 2).to_bytes(len(s) // 8, byteorder='big')


print("---------------- file content")
#with open("./data_05nov/data005.bin", mode='rb') as file: # b is important -> binary
#with open("./data_oct/data082.bin", mode='rb') as file: # b is important -> binary
#    fileContent = file.read()
fileContent = binascii.unhexlify('00FFFFFF803F684DFFFFFF856100')
print(type(fileContent))
print(fileContent)


# print("----------------")
byte_word = '11110100010111110101100111010111000100111001100110101110111101000101111101011001110101110010101010011000110101101111010001011111010110011101011101000001100101111111111011110100010111110101100111010111010110001001011100100110111101000101111101011001110101110110111110010110010011101111010001011111010110011101011110000110100101010111011011110100010111110101100111010111100111011001010010011110'
byte_array = bitstring_to_bytes(byte_word)
# print(type(byte_word))
# print(type(byte_array))
# print(byte_array)


print("---------------- b64 string")
b64_string = base64.b64encode(fileContent).decode('ascii')
print(type(b64_string))
print(b64_string)

print("---------------- decoded payload from file")
res = pd.decode_payload(b64_string)
print(type(res))
print(res)


print("---------------- decoded payload example byte word")
res = pd.decode_payload(byte_word)
print(type(res))
print(res)
