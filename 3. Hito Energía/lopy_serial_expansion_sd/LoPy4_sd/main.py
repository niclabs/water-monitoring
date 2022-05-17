def get_lorawan_maximum_payload_size(dr):
    mac_payload_size_dic = {'0':59, '1':59, '2':59, '3':123, '4':230, '5':230, '6':230}
    fhdr_size = 7   #in bytes. Assuming that FOpts length is zero
    fport_size = 1  #in bytes
    frm_payload_size = mac_payload_size_dic.get(str(dr)) - fhdr_size - fport_size
    return frm_payload_size

def get_data_pointers(buff):
    block_size = 51 # include 2 bytes for Checksum
    n_block = math.ceil(len(buff)/block_size)

    ptrs = list()
    for i in range(n_block):
        ptr_start = block_size*i

        if i == n_block-1:
            ptr_end = len(buff)-2
        else:
            ptr_end = block_size*(i+1)-2

        #if check_fletcher16(buff[ptr_start:ptr_end+2]):
        if mula(buff[ptr_start:ptr_end+2]):
            print("LoPy4 - PROCESS - Checksum OK")
            ptrs.append((ptr_start,ptr_end))
        else:
            print("LoPy4 - PROCESS - Checksum Error")

    print(ptrs)
    return ptrs

def mula(buff):
    return True

def get_file_size(filename):
    # https://docs.pycom.io/firmwareapi/micropython/uos/
    # Returns file size in bytes
    return os.stat(filename)[6]

def get_free_space(path):
    # Returns available space in KiB
    return os.getfree(path)

def file_exists_in_folder(filename, folder):
    lowercase_ls = list(map(lambda x: x.lower(), os.listdir(folder)))
    return filename.lower() in lowercase_ls

def save_text_file_sd(readings: list, index: int):
    '''
    Usage:
    file_index = save_text_file_sd(decoded_data, file_index)
    '''
    if get_free_space(SD_MOUNT_POINT) < 2:
        print("LoPy4 - SD - Too litle space available!")

    filename = ('data%d' % index) + '.csv'
    filepath = SD_MOUNT_POINT + '/' + filename
    if file_exists_in_folder(filename, SD_MOUNT_POINT) and get_file_size(filename) > MAX_FILE_SIZE:
        index += 1
        filepath = SD_MOUNT_POINT + ('/data%d' % index) + '.csv'
    #print('LoPy4 - SD dir contents before write:', os.listdir(SD_MOUNT_POINT))
    print("LoPy4 - SD - Writing in", filepath)
    with open(filepath, 'a') as f:
        for reading in readings:
            row = '%d;%d;%.1f\n' % reading
            f.write(row)
        print("LoPy4 - SD - Flushing")
        f.flush()
    os.sync()
    print("LoPy4 - SD - Filesystems Sync'd")
    #print('LoPy4 - SD dir contents after write:', os.listdir(SD_MOUNT_POINT))
    # Revisar lo escrito
    """
    with open(filepath, 'r') as f:
        print('LoPy4 - SD written content file:', f.read())
    """
    return index


print("LoPy4 - Initializing communication values...")


m_dr = DATA_RATE
print("LoPy4 - LoRaWAN - Initial Data Rate: ", m_dr)
lorawan_mtu = get_lorawan_maximum_payload_size(m_dr)
print("LoPy4 - LoRaWAN - Initial Maximum Payload Size: %d bytes" % lorawan_mtu)

print("LoPy4 - Starting Loop...")
a = 1

while True:
    print("*****************************")
    print("LoPy4 - SERIAL_RX - Cleaning Serial Rx buffer")
    n_bytes = uart.any()
    buff = uart.read(n_bytes)
    print("LoPy4 - SERIAL_RX - num bytes recv and dropped: %d" % n_bytes)
    print("LoPy4 - Loop %d" % a)
    a=a+1
    print("LoPy4 - Going to sleep...")
    time.sleep(3)
    machine.sleep(50 * 1000) # 50 s

    print("Wake up...")

    # Se lee el mensaje serial proveniente desde el Arduino
    start = time.ticks_ms()
    recv_bytes = bytes()
    n_bytes = 0
    recv_len = -1
    while True:
        if time.ticks_diff(time.ticks_ms(), start) > 3000:
            break
        n_bytes = uart.any()
        if n_bytes != 0:
            recv_bytes = recv_bytes + uart.read(n_bytes)
            recv_len = len(recv_bytes)
            print("LoPy4 - SERIAL_RX - num bytes recv: %d" % n_bytes)
            print("LoPy4 - SERIAL_RX - bytes recv: %s" % ubinascii.hexlify(recv_bytes))
            print("LoPy4 - SERIAL_RX - total recv_bytes: %d" % recv_len)

    if(recv_len > 0):
        print("LoPy4 - SERIAL_RX - Dropping 1st byte (Dummy RX)")
        recv_bytes = recv_bytes[1:] # Ahora sí son el mismo mensaje
        print("LoPy4 - SERIAL_RX - actual recv msg (%d bytes): %s" % (recv_len, ubinascii.hexlify(recv_bytes)))
        print("LoPy4 - DECODED : ", end='')
        decoded_data = decode_payload(recv_bytes)
        print(decoded_data)
        file_index = save_text_file_sd(decoded_data, file_index)

        # se obtiene el tamaño máximo disponible para un mensaje LoRaWAN
        print("LoPy4 - LoRaWAN - Data Rate: ", m_dr)
        lorawan_mtu = get_lorawan_maximum_payload_size(m_dr)
        print("LoPy4 - LoRaWAN - Maximum Payload Size: %d bytes" % lorawan_mtu)


        ptrs = get_data_pointers(recv_bytes)

        block_size = 49     # bytes
        blocks_per_mtu = lorawan_mtu//block_size    # n° blocks per a LoRaWAN message
        n_lorawan_messages = math.ceil(len(ptrs)/blocks_per_mtu)
        n_blocks_in_full_lorawan_msg = blocks_per_mtu * (len(ptrs)//blocks_per_mtu)
        print("LoPy4 - LoRaWAN - The current LoRaWAN MTU supports %d blocks" % blocks_per_mtu)
        print("LoPy4 - LoRaWAN - The %d blocks are sent in multiple (%d) LoRaWAN messages" % (len(ptrs),n_lorawan_messages))


        print("LoPy4 - LoRaWAN - blocks_per_mtu: %d" % blocks_per_mtu)
        print("LoPy4 - LoRaWAN - len(ptrs): %d" % len(ptrs))
        print("LoPy4 - LoRaWAN - n_blocks_in_full_lorawan_msg: %d" % n_blocks_in_full_lorawan_msg)

        i = 0
        while i < n_blocks_in_full_lorawan_msg:
            aux = bytearray()
            for _ in range(blocks_per_mtu):
                aux += recv_bytes[ptrs[i][0]:ptrs[i][1]]
                i += 1
            try:
                s.setblocking(True)
                print("LoPy4 - LoRaWAN - Sending %d bytes" % len(aux))
                s.send(aux)
                s.setblocking(False)
            except OSError as e:
                if e.args[0] == 11:
                    s.setblocking(False)
                    print("LoPy4 - LoRaWAN_ERROR - It can probably be a duty cycle problem")
        if n_blocks_in_full_lorawan_msg != len(ptrs):
            aux = bytearray()
            for i in range(n_blocks_in_full_lorawan_msg, len(ptrs)):
                aux += recv_bytes[ptrs[i][0]:ptrs[i][1]]
            try:
                s.setblocking(True)
                print("LoPy4 - LoRaWAN - Sending %d bytes" % len(aux))
                s.send(aux)
                s.setblocking(False)
            except OSError as e:
                if e.args[0] == 11:
                    s.setblocking(False)
                    print("LoPy4 - LoRaWAN_ERROR - It can probably be a duty cycle problem")
