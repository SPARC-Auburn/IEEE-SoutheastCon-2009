import serial






def simple(self):
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

trimmed()
