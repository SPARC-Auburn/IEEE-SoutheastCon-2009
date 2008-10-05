import serial
import time

class Laser(serial.Serial):
	"""Laser class, extends serial so you can use it like a serial port.
	
	The Laser class extends the serial class so you can create it
	and pass it the serial port to it at the same time.
	i.e.: laser1 = laser.Laser("/dev/ttyACM0")
	
	You can perform any function on it that you can perform on a serial object as well as, scan()."""
	
	def __init__(self, serialPort):
        	"""Calls the Serial __init__ and then sets timeout and other default options"""
		serial.Serial.__init__(self, serialPort)
		self.timeout = .001
		self.baudrate = 115200
		# Prime the LRF (this is done because of first read errors using a low timeout time)
		self.write('G00076800\r')
		self.clear()
		
	def scan(self, start = "000", stop = "768", step = "00"):
		"""This function will scan from start to stop, returning a reading for every step number of scans.
		
		This function allows for specifying start, stop, and step values,
		but also provides default values, so all parameters are optional.
		This function will return an array with a length equal to the number scans,
		with each element in the array containing the corresponding reading.
		i.e.: readings[i] is equal to reading of the ith+1 scan."""
		result = []
		# Generate the command
		command = 'G'+start+stop+step+'\r'
		
		# Send the command
		self.write(command)
		
		# Retrieve data
		# Burn the first two lines.
		self.readline()
		self.readline()
		
		line = self.readline()
		# While there is no empty line return, ('\n'), continue processing data
		while (line != '\n'):
			# While we haven't reached the end of the line
			i = 0
			while (line[i] != '\n'):
				dist = (ord(line[i]) - 48) << 6
				i += 1
				dist += ord(line[i]) - 48
				i += 1
				result.append(dist)
			line = self.readline()
			
		# Return the resulting list of distances
		return result


	def safe_scan(self, start = "000", stop = "768", step = "00"):
		"""Checks input and then passes to scan()."""
		# Check input
		try:
			istart = int(start)
			istop = int(stop)
			istep = int(step)
		except ValueError:
			print "Error converting input, make sure that all parameters are positive integers."
			return []

		# Validate input
		if istart <= 0 and istart < istop and istop <= 768:
			return self.scan(start, stop, step)
		return []

	def clear(self):
		"""This function clears all input/output buffers from the serial device."""
		time.sleep(.01)
		self.flushOutput()
		self.flushInput()
