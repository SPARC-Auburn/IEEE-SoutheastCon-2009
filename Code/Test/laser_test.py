import serial






def simple():
	sio = serial.Serial("/dev/ttyACM0")
	sio.timeout = 2
	sio.write("G00076800\r")
	
	line = sio.readline()
	while (line != ''):
		print line
		line = sio.readline()
	
	sio.close()


def trimmed():
	sio = serial.Serial("/dev/ttyACM0")
	sio.timeout = 2

	for i in range(0,10):
		print i,
		sio.write("G00076800\r")
		
		sio.readline()
		sio.readline()
		
		data = sio.read(1564)
		sio.flushInput()

def granual():
	sio = serial.Serial("/dev/ttyACM0")
	sio.timeout = 2
	
	sio.write("G00076800\r")
	sio.readline()
	sio.readline()
	
	line1 = sio.read()
	line2 = sio.read()
	while((line1 != '\n') or (line2 != '\n')):
		if(not((line1 == '0') and (line2 == 'C'))):
			if(line1 != '\n'):
				print line1,
			if(line2 != '\n'):
				print line2,
			print ' ',
		line1 = sio.read()
		line2 = sio.read()
	sio.close()

#trimmed()
granual()
