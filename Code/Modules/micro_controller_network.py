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
	* Controlling lights and switches
	* Handleing Interrupts from Master Node.
	'''

# Imports #
# Logging
import logging
log = logging.getLogger('MCN')
master_log = logging.getLogger('Master Node')
# Configs
import configs
import configobj
global config, enable
config = configs.get_config('Micro Controller Network')
enabled = config['enabled']
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
			micro_controllers[x] = MicroController(serial = config[x]['serial_port'], baud_rate = config[x]['baud_rate'])
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
	def __init__(self, serial='', baud_rate = 115200):
		'''
			Constructor - opens serial port given and initializes the master node.
			'''
		global enabled
		self.serial = Serial()
		self.serial.port = serial
		self.serial.baudrate = baud_rate
		self.serial.timeout = 1
		if enabled:
			try:
				self.serial.open()
			except Exception as e:
				log.error("Unable to open serial port %s: %s" % (serial, e))
		self.send_lock = Lock()
		self.debugging = debug(self.serial)
		if self.serial.isOpen():
			self.debugging.start()
		return
	
	def send(self, msg):
		''''
			Sends a msg to a micro controller at the given address.
			Address must be a byte.
			msg must be a list of bytes to send.
			Bytes must be a 2 char string representing two hex digits.
			'''
		# Checks
		if not enabled:
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
		self.debugging.shutdown()
		self.debugging.join()
		return
		
		
class debug(Thread):
	def __init__(self, serial):
		Thread.__init__(self)
		self.serial = serial
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
		if len(input) > 2:
			log.debug("MCN: %s" % input)
		return
		