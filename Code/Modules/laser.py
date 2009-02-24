#
#  laser.py
#  
#
#  Created by William Woodall on 2/24/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#


'''
Module for controlling the Hokuyo URG-04LX Laser Range Finder.
'''

# Imports #
# Configs
import configs
config = configs.get_config('Laser Range Finder')
enabled = config['enabled']
# Logging
import logging
log = logging.getLogger(config['logger_name'])
# PySerial
if enabled:
	try:
		from serial import Serial
	except Exception as e:
		log.error("Serial cannot be imported, you may need to install it: %s" % e)
		sys.exit(1)
else:
	log.warning("The Laser Range Finder is disabled and will quitely do nothing.")
	
# Static Functions #
def init():
	global initialized, lrfs
	lrfs = {}
	lrfs['Laser Range Finder'] = LaserRangeFinder(config = config)
	initialized = True
	
def get_object(id = 'Laser Range Finder'):
	global initialized, lrfs
	if not initialized:
		log.critical("The laser.init() method has to be called before retrieving objects.")
		sys.exit(1)
	try:
		return lrfs[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available Laser Range Finders.  Please double check the names in the config files.")
		
# Classes #
class LaserRangeFinder:
	'''
	This class represents and allows access to a Hukuyo URG-04LX Laser Range Finder.
	'''
	def __init__(self, serial = '/dev/ttyACM0', baud_rate = '115200', start = 000, stop = 768, step = 00, config = None):
		if config:
			self.config = config
			self.serial_port = config['serial_port']
			self.baud_rate = config['baud_rate']
			self.start = config['start']
			self.stop = config['stop']
			self.step = config['step']
		else:
			self.config = None
			self.serial_port = serial
			self.baud_rate = baud_rate
			self.start = start
			self.stop = stop
			self.step = step
		if enabled:
			self.serial = Serial()
			self.serial.port = self.serial_port
			self.serial.baud_rate = self.baud_rate
			self.serial.timeout = 1
			try:
				self.serial.open()
			except Exception as e:
				log.error("Unabled to open the serial port %s: %s" % (self.serial_port, e))
		#End __init__
		
	def scan(self):
		"""This function will scan from start to stop, returning a reading for every step number of scans.
		
		This function allows for specifying start, stop, and step values,
		but also provides default values, so all parameters are optional.
		This function will return an array with a length equal to the number scans,
		with each element in the array containing the corresponding reading.
		i.e.: readings[i] is equal to reading of the ith+1 scan."""
		result = []
		# Generate the command
		command = 'G'+self.start+self.stop+self.step+'\r'
		
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
		"""Checks input and then passes to scan(). DEPRECIATED"""
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
		self.flushOutput()
		self.flushInput()
		
	def shutdown(self):
		self.serial.close()
		
		
		
		
		
		
		
		
		
		
		