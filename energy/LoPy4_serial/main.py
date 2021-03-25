def get_lorawan_maximum_payload_size(dr):
    mac_payload_size_dic = {'0':59, '1':59, '2':59, '3':123, '4':230, '5':230, '6':230}
    fhdr_size = 7   #in bytes. Assuming that FOpts length is zero
    fport_size = 1  #in bytes
    frm_payload_size = mac_payload_size_dic.get(str(dr)) - fhdr_size - fport_size
    return frm_payload_size

def checksum_validator(buff):
    return True

def remove_checksum_bytes(buff):
    block_size = 51 # include 2 bytes for Checksum
    n_block = math.ceil(len(buff)/block_size)

    ptrs = list()
    for i in range(n_block):
        ptr_start = block_size*i

        if i == n_block-1:
            ptr_end = len(buff)-2
        else:
            ptr_end = block_size*(i+1)-2

        if checksum_validator(buff[ptr_start:ptr_end]):
            print("LoPy4 - PROCESS - Checksum OK")
            ptrs.append((ptr_start,ptr_end))
        else:
            print("LoPy4 - PROCESS - Checksum Error")

    print(ptrs)
    return ptrs






print("LoPy4 - Initializing communication values...")
print("LoPy4 - SERIAL_RX - Cleaning Serial Rx buffer")
n_bytes = uart.any()
buff = uart.read(n_bytes)
print("LoPy4 - SERIAL_RX - num bytes recv and dropped: %d" % n_bytes)

#s.send(bytes([0xff]))
#m_stats = lora.stats()
#m_dr = m_stats[4]
m_dr = DATA_RATE
print("LoPy4 - LoRaWAN - Initial Data Rate: ", m_dr)
lorawan_mtu = get_lorawan_maximum_payload_size(m_dr)
print("LoPy4 - LoRaWAN - Initial Maximum Payload Size: %d bytes" % lorawan_mtu)


print("LoPy4 - Starting Loop...")
a = 1

while True:
    print("*****************************")
    print("LoPy4 - Loop %d" % a)
    a=a+1
    print("LoPy4 - Going to sleep...")
    #time.sleep(60)
    machine.sleep(100000)

    print("Weak up...")

    # Se lee el mensaje serial proveniente desde el Arduino
    start = time.ticks_ms()
    recv_bytes = bytes()
    n_bytes = 0
    while True:
        if time.ticks_diff(time.ticks_ms(), start) > 3000:
            break
        n_bytes = uart.any()
        if n_bytes != 0:
            recv_bytes = recv_bytes + uart.read(n_bytes)
            print("LoPy4 - SERIAL_RX - num bytes recv: %d" % n_bytes)
            print("LoPy4 - SERIAL_RX - bytes recv: %s" % ubinascii.hexlify(recv_bytes))

    if(len(recv_bytes) != 0):
        # se obtiene el tamaño máximo disponible para un mensaje LoRaWAN
        print("LoPy4 - LoRaWAN - Data Rate: ", m_dr)
        lorawan_mtu = get_lorawan_maximum_payload_size(m_dr)
        print("LoPy4 - LoRaWAN - Maximum Payload Size: %d bytes" % lorawan_mtu)


        ptrs = remove_checksum_bytes(recv_bytes)

        measure_size = 7    # bytes
        block_size = 51     # bytes
        n_availables_measures = lorawan_mtu//measure_size
        n_measures = (len(recv_bytes) - math.ceil(len(recv_bytes)/block_size)*2)/measure_size

        print("LoPy4 - LoRaWAN - The current LoRaWAN MTU supports %d measures" % n_availables_measures)
        print("LoPy4 - LoRaWAN - The current serial message has %d measures of 7 bytes each" % n_measures)
        if(n_measures <= n_availables_measures):
            # el tamaño de la MTU permite enviar todos los payloads de una vez
            print("LoPy4 - LoRaWAN - The %d measures are sent in only a LoRaWAN message" % n_measures)
            aux = bytes()
            for j in ptrs:
                aux = aux + recv_bytes[j[0]:j[1]]
            s.send(bytes(aux))
        else:
            # el tamaño de la MTU NO permite enviar todos los payloads de una vez.
            # Se deben enviar varios mensajes LoRaWAN
            n_lorawan_messages = math.ceil(n_measures/n_availables_measures)
            print("LoPy4 - LoRaWAN - The %d measures are sent in multiple (%d) LoRaWAN messages" % (n_measures,n_lorawan_messages))
            aux = bytes()
            for k in ptrs:
                aux = aux + recv_bytes[k[0]:k[1]]
            for x in range(1,n_lorawan_messages+1):
                pass
                #s.send(bytes(aux[(x-1)*n_availables_measures*measure_size:x*n_availables_measures*measure_size]))

        m_stats = lora.stats()
        m_dr = m_stats[4]
