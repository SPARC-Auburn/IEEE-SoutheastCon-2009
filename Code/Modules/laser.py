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
datalog = logging.getLogger("Dash_log")
#serverHandler = LoggingServerHandler('', PORT)
#dash_log.addHandler(serverHandler)

try:
	log = logging.getLogger(config['logger_name'])
except NameError:
	pass
# Events
import events
# Threading
from threading import Event, Thread
# Math
import math
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
			self.monitor_event = Event()
			self.monitor = LaserMonitor(self)
			self.monitor.start()
			self.set_monitor_settings()
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
		command = 'G'+start+stop+step+'\r'
		
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
				
	def check_for_obj(self):
		data = self.scan()[self.mstart:self.mstop]
		array = [self.mstart]
		array.extend(data)
		print array
		datalog.info(toStringFormat('Sorter', 'Position', array))
		
		for x in range(len(data)):
			if data[x] > self.mrange:
				data[x] = 0
		
		potential_spikes = []
		for x in range(1, len(data)):
			if abs(data[x] - data[x-1]) > self.mspike:
				potential_spikes.append(x)
		
		spikes = []
		s1 = None
		for x in range(len(potential_spikes)-1):
			if potential_spikes[x+1] - potential_spikes[x] >= self.mwidth:
				if s1 is None:
					s1 = potential_spikes[x]
				else:
					spikes.append((s1, potential_spikes[x]))
					s1 = None
		if s1 != None:
			spikes.append((s1, potential_spikes[len(potential_spikes)-1]))
			s1 = None
		
		result = []
		for x,y in spikes:
			mid = (x+y)/2
			if data[mid-1] > 0:
				log.debug("Object from Laser (r, theta): %d, %d" % (data[mid-1], (mid+self.mstart-384)/3))
				temp_r, temp_t = self.translate_to_robot(data[mid-1], mid+self.mstart-384)
				result.append((temp_r, temp_t))
		return result

	def translate_to_robot(self, r, t):
		vlrx = 127.47
		vlry = 120.65
		t_r = math.radians(t/3)
		vloy = math.cos(t_r)*r
		vlox = math.sin(t_r)*r
		vrox = vlrx + vlox
		vroy = vlry + vloy
		# convert back to polar
		theta = math.atan(vrox/vroy)
		theta = math.degrees(theta)
		magnitude = math.sqrt(math.pow(vroy, 2) + math.pow(vrox, 2))
		return magnitude, theta

	def set_monitor_settings(self, angle = 30, range = 600, spike = 50, width = 5):
		self.mstart = 384-(angle*3)
		self.mstop = 384+(angle*3)
		self.mrange = range
		self.mspike = spike
		self.mwidth = width
		
	def monitor_obstacles(self):
		self.monitor_event.set()
	
	def clear(self):
		"""This function clears all input/output buffers from the serial device."""
		if not enabled:
			return
		if self.serial.isOpen():
			self.serial.flushOutput()
			self.serial.flushInput()
		
	def shutdown(self):
		log.info("Laser Range Finder shutting down.")
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
		self.stop = False
		
	def run(self):
		while not self.stop:
			self.lrf.monitor_event.wait(1)
			if self.lrf.monitor_event.isSet():
				objects = self.lrf.check_for_obj()
				if objects != []:
					log.debug("LRF Detected Object(s): %s" % str(objects))
					events.triggerEvent('LRF Detected Object', objects)
					self.lrf.monitor_event.clear()
		return
					
		
	def shutdown(self):
		self.stop = True
