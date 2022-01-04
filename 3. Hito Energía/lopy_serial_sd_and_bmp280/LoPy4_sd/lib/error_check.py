def checksum_raw_fletcher16(message):
    """
    Fletcher16 check function. Receives a message with a checksum appended and returns 0 if message is correct and another value if not.
    """
    sum1 = 0
    sum2 = 0
    for byte in message:
        sum1 = (sum1 + byte)
        sum2 = (sum1 + sum2)
    sum1 %= 255
    sum2 %= 255
    return (sum2 << 8) | sum1

def get_check_bytes(checksum_raw):
    """
    Given a raw Fletcher16 checksum, returns the two corresponding check bytes in an array of bytes.
    """
    f0 = checksum_raw & 0xff
    f1 = (checksum_raw >> 8) & 0xff
    c0 = 0xff - ((f0 + f1) % 0xff)
    c1 = 0xff - ((f0 + c0) % 0xff)
    return bytes([c0, c1])

def check_fletcher16(message_with_checksum):
    """
    Given a message of bytes with it corresponding Fletcher16 checksum appended at the end in the form b1b2...bNc1c2, verifies if the
    checksum corresponds to the message.
    Returns True if message and checksum correspond, and False if not.
    """
    if(checksum_raw_fletcher16(message_with_checksum) == 0):
        return True
    return False