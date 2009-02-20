import serial
import sys

serial = serial.Serial()
serial.port = sys.argv[1]
serial.baudrate = sys.argv[2]
serial.timeout = 1
serial.open()

def write_to_ee(address,msg):
    message = ''
    message = []
    
    message.append('\x31')     # Send the opcode for EEPROM Write
    addr = dec2hex(int(address))
    if len(addr) < 2:               # If the address isn't being sent in 2 bytes
        addr =  '\x00' + addr       # Pack another byte on the front
        message.append(addr[0:1])       # Send two bytes for the EEPROM Address 
        message.append(addr[1:2])
    else:
        message.append(addr)

    count = 0
    for x in msg:
        if int(x) > 255:
            count += 2
        else:
            count += 1

    message.append(dec2hex(count))

    for x in msg:
        if int(x) > 255:            # If the next number is an int, break it up
            temp = dec2hex(int(x))
            message.append(temp[0:1])
            message.append(temp[1:2])
        else:
            message.append(dec2hex(int(x)))     # Otherwise print the next character
    print message
    send(message)                               # Send the message
    return
    
def read_from_ee(address):
    message = ''
    message = []

    message.append(dec2hex(32))     # Send the opcode for EEPROM Read
    addr = dec2hex(int(address))
    if len(addr) < 2:               # If the address isn't being sent in 2 bytes
        addr =  '\x00' + addr       # Pack another byte on the front
        message.append(addr[0:1])       # Send two bytes for the EEPROM Address
        message.append(addr[1:2])
    else:
        message.append(addr)
    
    if serial.isOpen():
        msg = serial.read()
    msg.strip()
    if msg[0:1] == '\x30':
        return msg[1:2].decode('hex')
    else:
        return '0'


def send(msg):
    message = ''
    for x in msg:
        message += x

    print "Sending message: %s " % message.encode('hex')
    
    if serial.isOpen():
        serial.write(message + '\r')

    return

def dec2hex(dec):
    '''
        Converts a given decimal number to hex.
        '''
    hex = '%X' % dec
    if len(hex)%2 is 1: #Prevents Odd-length error with decode.
        hex = '0'+hex
    return hex.decode("hex")
