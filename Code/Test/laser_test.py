import serial

sio = serial.Serial("/dev/ttyACM0")
sio.timeout = 2
sio.write("G00076800\r")

line = sio.readline()
while (line != ''):
	print line
	line = sio.readline()

sio.close()
