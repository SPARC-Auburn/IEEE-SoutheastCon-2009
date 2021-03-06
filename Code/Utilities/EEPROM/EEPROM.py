import serial
import sys

serial = serial.Serial()
if len(sys.argv) > 1:
	serial.port = sys.argv[1]
else:
	serial.port = '/dev/ttyUSB0'
if len(sys.argv) > 2:
	serial.baudrate = sys.argv[2]
else:
	serial.baudrate = 115200
serial.timeout = 1
serial.open()
	
def write_to_ee(address,values):
	if type(values) != list:
		print 'Error, values must be a list of ints'
	msg = '\x01'
	msg += dec2hex(address)
	number = len(values) * 2
	msg += dec2hex(number)[1:2]
	for x in range(number/2):
		msg += dec2hex(values[x])
	if serial.isOpen():
		print "Writing:", msg.encode('hex')
		serial.write(msg)
	else:
		print 'Error, the serial port is not open.'
	return

def read_from_ee(address):
	value = read_hex_from_ee(address)
	if value is None:
		return
	value = hex2dec(value[4:6])
	return value

def read_hex_from_ee(address):
	serial.flushOutput()
	serial.flushInput()
	msg = '\x02'
	msg += dec2hex(address)
	if serial.isOpen():
		serial.write(msg)
	else:
		print 'Error, the serial port is not open.'
		return
	value = serial.readline()
	if value == '':
		print 'Error, did not receive response from the Micro Controller.'
		return None
	value = value.strip()
	return value

def hex2dec(hex):
	return int(hex, 16)

def dec2hex(dec):
	'''
	Converts a given decimal number to hex.
	'''
	hex = '%X' % dec
	while len(hex) < 4:
		hex = '0' + hex
	hex = hex.decode("hex")
	if len(hex) == 1:
		hex = '\x00' + hex
	return hex


