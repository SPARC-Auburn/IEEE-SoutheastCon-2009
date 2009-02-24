#
#  micro_controller_network.py
#  
#
#  Created by William Woodall on 1/29/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
'''
	Module for interfacing with the Micro Controller Network.
	TODO:
	'''

# Imports #
# Configs
import configs
import configobj
global config, enabled
config = configs.get_config('Micro Controller Network')
enabled = config['enabled']
# Logging
import logging
log = logging.getLogger(config['logger_name'])
# Threading
from threading import Thread, Lock
# PySerial
if enabled:
	try:
		from serial import Serial
	except Exception as e:
		log.error("Serial cannot be imported, you may need to install it: %s" % e)
else:
	log.warning("The Micro Controller Network is Disabled and all connect Equiptment will not work!")

# Static Functions #
def init():
	global config, initialized, micro_controllers
	micro_controllers = {}
	for x in config:
		if type(config[x]) is configobj.Section:
			micro_controllers[x] = MicroController(x, config[x])
	initialized = True
	
def get_object(id):
	global micro_controllers, initialized
	if not initialized:
		log.critial("The micro_controller_network.init() method has to be called before retrieving objects.")
		exit(1)
	try:
		return micro_controllers[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available Micro Controllers.  Please double check the names in the config files.")
		
# Classes #
class MicroController:
	'''
		This is the class that represents and provides access to a micro controller 
		of a micro controller network.
		'''
	def __init__(self, name, config = None):
		'''
			Constructor - opens serial port given and initializes the micro controller.
			'''
		#Name
		self.name = name
		self.config = config
		#Log
		self.log = logging.getLogger(config['logger_name'])
		#Serial
		self.serial = Serial()
		self.serial.port = self.config['serial_port']
		self.serial.baudrate = self.config['baud_rate']
		self.serial.timeout = 1
		self.enabled = config['enabled']
		if self.enabled:
			try:
				self.serial.open()
			except Exception as e:
				self.log.error("Unable to open serial port %s: %s" % (self.serial, e))
		self.send_lock = Lock()
		#Services
		self.services = []
		#Threading
		self.debugging = debug(self.name, self)
		if self.serial.isOpen():
			self.debugging.start()
		return
		
	def register(self, obj):
		self.services.append(obj)
		
	def handleMsg(self, msg):
		for service in self.services:
			for return_code in service.return_codes:
				if msg[0:1] == return_code:
					service.notify(service.return_codes[return_code], msg)
					return
		self.log.error("Unregistered return code received: %s", msg)
		return
	
	def send(self, msg):
		'''
			Sends a msg to a micro controller at the given address.
			Address must be a byte.
			msg must be a list of bytes to send.
			Bytes must be a 2 char string representing two hex digits.
			'''
		if not self.enabled:
			return
		if type(msg) is list:
			self.send_list_of_bytes(msg)
		else:
			self.serial.write(msg)
	
	def send_list_of_bytes(self, msg):
		'''
			Sends a list of bytes to the send method, legacy.
			'''
		# Checks
		if not self.enabled:
			return
		if type(msg) is not list:
			self.log.error("Messages sent to the micro controller must be a list of bytes!")
			return
		
		# Implementation
		message = ''
		for x in msg:
			message += x
		self.send_lock.acquire()
		self.log.debug("Sending message: %s" % message.encode('hex'))
		if self.serial.isOpen():
			self.serial.write(message+'\r')
		self.send_lock.release()
		return
		
	def shutdown(self):
		self.log.info("Micro Controller %s is shutting down.", self.name)
		for service in self.services:
			service.shutdown()
		self.debugging.shutdown()
		if self.debugging.is_alive():
			self.debugging.join()
		return
		
		
class debug(Thread):
	def __init__(self, name, mc):
		Thread.__init__(self)
		self.name = name
		self.mc = mc
		self.serial = mc.serial
		self.stop = False
		
	def run(self):
		input = ''
		while not self.stop:
			if self.serial.isOpen():
				input = self.serial.readline()
			self.processInput(input)
			
	def shutdown(self):
		self.stop = True
		return
	
	def processInput(self, input):
		input = input.strip()
		if len(input) > 2 and input[0:2] == 'RST':
			self.mc.log.info("Micro Controller %s reset.", self.name)
		elif input != '' and input != '\r' and input != '\n':
			self.mc.handleMsg(input)
		return
		
