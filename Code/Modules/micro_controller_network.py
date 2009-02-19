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
# Logging
import logging
log = logging.getLogger('MCN')
master_log = logging.getLogger('Master Node')
# Configs
import configs
import configobj
global config, enabled
config = configs.get_config('Micro Controller Network')
enabled = config['enabled']
# Threading
from threading import Thread, Lock
# Queue
import Queue
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
			micro_controllers[x] = MicroController(name = x, serial = config[x]['serial_port'], baud_rate = config[x]['baud_rate'])
	initialized = True
	
def get_object(id):
	global micro_controllers, initialized
	if not initialized:
		log.critial("The micro_controller_network.init() method has to be called before retrieving objects.")
		exit(1)
	try:
		return micro_controllers[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available Master Nodes.  Please double check the names in the config files.")
		
# Classes #
class MicroController:
	'''
		This is the class that represents and provides access to the master 
		node of a micro controller network.
		'''
	def __init__(self, name = None, serial='', baud_rate = 115200):
		'''
			Constructor - opens serial port given and initializes the master node.
			'''
		global config
		#Name
		self.name = name
		if name is None:
			name = 'Unamed micro controller'
		#Serial
		self.serial = Serial()
		self.serial.port = serial
		self.serial.baudrate = baud_rate
		self.serial.timeout = 1
		self.enabled = config[name]['enabled']
		if self.enabled:
			try:
				self.serial.open()
			except Exception as e:
				log.error("Unable to open serial port %s: %s" % (serial, e))
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
		log.error("Unregistered return code received: %s", msg)
		return
	
	def send(self, msg):
		''''
			Sends a msg to a micro controller at the given address.
			Address must be a byte.
			msg must be a list of bytes to send.
			Bytes must be a 2 char string representing two hex digits.
			'''
		# Checks
		if not self.enabled:
			return
		if type(msg) is not list:
			log.error("Messages sent to the micro controller must be a list of bytes!")
			return
		
		# Implementation
		message = ''
		for x in msg:
			message += x
		self.send_lock.acquire()
		master_log.debug("Sending message: %s" % message.encode('hex'))
		if self.serial.isOpen():
			self.serial.write(message+'\r')
		self.send_lock.release()
		return
		
	def shutdown(self):
		log.info("Micro Controller %s is shutting down.", self.name)
		self.debugging.shutdown()
		if self.debugging.is_alive():
			self.debugging.join()
		return
		
		
class debug(Thread):
	def __init__(self, name, mc):
		Thread.__init__(self)
		self.name = name
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
		if input[0:2] == 'RST':
			log.info("Micro Controller %s reset.", self.name)
		else
			mc.handleMsg(input)
		return
		