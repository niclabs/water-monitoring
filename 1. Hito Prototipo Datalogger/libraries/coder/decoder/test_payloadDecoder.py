import payloadDecoder as pd
import unittest
import base64

def bitstring_to_bytes(s):
    return int(s, 2).to_bytes(len(s) // 8, byteorder='big')

class TestBinaryDecoding(unittest.TestCase):

    def testPositiveValues(self):
        byte_word = '11110010010111110101101011101000100000110000000101101111111100100101111101011010111010001000100000000001100000001111001001011111010110101110100010001101000000011001000111110010010111110101101011101000100100100000000110100010111100100101111101011010111010001001011100000001101100111111001001011111010110101110100010011100000000011100010011110010010111110101101011101000101000010000000111010101'
        byte_array = bitstring_to_bytes(byte_word)
        b64_string = base64.b64encode(byte_array).decode('ascii')
        golden =  [(2,1599793283,36.7),
                  (2,1599793288,38.4),
                  (2,1599793293,40.1),
                  (2,1599793298,41.8),
                  (2,1599793303,43.5),
                  (2,1599793308,45.2),
                  (2,1599793313,46.9)]
        res = pd.decode_payload(b64_string)
        self.assertEqual(golden, res)
    
    def testNegativeValues(self):
        byte_word = '11110100010111110101100111010111000100111001100110101110111101000101111101011001110101110010101010011000110101101111010001011111010110011101011101000001100101111111111011110100010111110101100111010111010110001001011100100110111101000101111101011001110101110110111110010110010011101111010001011111010110011101011110000110100101010111011011110100010111110101100111010111100111011001010010011110'
        byte_array = bitstring_to_bytes(byte_word)
        b64_string = base64.b64encode(byte_array).decode('ascii')
        golden =  [  (4, 1599723283, -657.4),
                     (4, 1599723306, -635.8),
                     (4, 1599723329, -614.2),
                     (4, 1599723352, -592.6),
                     (4, 1599723375, -571),
                     (4, 1599723398, -549.4),
                     (4, 1599723421, -527.8),
                ]
        res = pd.decode_payload(b64_string)
        self.assertEqual(golden, res)
    
    def testSingleValue(self):
        byte_word = '11110100010111110101100111010111000100110000110100000101000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'
        byte_array = bitstring_to_bytes(byte_word)
        b64_string = base64.b64encode(byte_array).decode('ascii')
        golden =  [(4, 1599723283, 333.3)]
        res = pd.decode_payload(b64_string)
        self.assertEqual(golden, res)

    

if __name__ == '__main__':
    unittest.main()