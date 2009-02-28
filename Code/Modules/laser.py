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
try:
	config = configs.get_config('Laser Range Finder')
	enabled = config['enabled']
except NameError:
	enabled = True
# Logging
import logging
try:
	log = logging.getLogger(config['logger_name'])
except NameError:
	pass
# Events
import events
# Threading
from threading import Event, Thread
# Psyco
try:
	import psyco
	psyco.full()
except ImportError:
	pass
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
			#Threading
			self.check_for_obj = Event()
			self.monitor = LaserMonitor(self)
			self.monitor.start()
		#End __init__
		
	def scan(self, start = None, stop = None, step = None):
		"""
		This function will scan from start to stop, returning a reading for every step number of scans.
		"""
		if not enabled:
			return
		if start is None:
			start = self.start
		if stop is None:
			stop = self.stop
		if step is None:
			step = self.step
		result = []
		# Generate the command
		command = 'G'+self.start+self.stop+self.step+'\r'
		
		# Send the command
		self.serial.write(command)
		
		# Retrieve data
		# Burn the first two lines.
		self.serial.readline()
		self.serial.readline()
		
		line = self.serial.readline()
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
			line = self.serial.readline()
			
		# Return the resulting list of distances
		return result

	def monitor_settings(self, start = 294, stop = 474, range = 600, spike = 50, width = 5):
		self.monitor.start = start
		self.monitor.stop = stop
		self.monitor.range = range
		self.monitor.spike = spike
		self.monitor.width = width
		
	def monitor_obstacles(self):
		self.check_for_obj.set()
	
	def clear(self):
		"""This function clears all input/output buffers from the serial device."""
		if not enabled:
			return
		if self.serial.isOpen():
			self.serial.flushOutput()
			self.serial.flushInput()
		
	def shutdown(self):
		if not enabled:
			return
		if self.serial.isOpen():
			self.serial.close()
		self.monitor.shutdown()
		self.monitor.join()
		
		
		
class LaserMonitor(Thread):
	def __init__(self, lrf):
		Thread.__init__(self)
		self.name = 'Laser Monitor'
		self.lrf = lrf
		self.serial = lrf.serial
		self.check_for_obj = lrf.check_for_obj
		self.stop = False
		self.start_step = 294 # Can't use self.start
		self.stop_step = 474
		self.range = 600
		self.spike = 50
		self.width = 5
		
	def run(self):
		while not self.stop:
			self.check_for_obj.wait(1)
			if self.check_for_obj.isSet():
				# scan
				data = scan(self.start_step, self.stop_step, '00')
				# Cut off things over the range
				for x in range(len(data)):
					if data[x] > self.range:
						data[x] = 0
				# Check for spikes
				spikes = []
				for x in range(1, len(data)):
					if abs(data[x] - data[x-1]) > self.spike:
						spikes.append(x)
				# Check for objects
				objects = []
				obj_start = None
				for x in range(len(spikes)-1):
					if spikes[x+1] - spikes[x] >= self.width:
						if obj_start is None:
							obj_start = spikes[x]
						else:
							objects.append((obj_start, spikes[x]))
							obj_start = None
				if obj_start != None:
					objects.append((obj_start, spikes[len(spikes)-1]))
					obj_start = None
				
				result = []
				for x,y in objects:
					mid = (x+y)/2
					result.append((data[mid], mid+start))
				if result != []:
					events.triggerEvent('LRF Object Detected', '0'+result)
					log.debug("Object(s) Detected by LRF: %s" % str(result))
					check_for_obj.clear()
		return
		
	def shutdown(self):
		self.stop = True
