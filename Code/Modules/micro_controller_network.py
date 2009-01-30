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
global config, enable
config = configs.get_config('Micro Controller Network')
enabled = config['enabled']
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
	global config, initialized, master_nodes
	master_nodes = {}
	master_nodes['master'] = MasterNode(serial = config['serial_port'])
	initialized = True
	
def get_object(id = 'master'):
	global master_nodes, initialized
	if not initialized:
		log.critial("The micro_controller_network.init() method has to be called before retrieving objects.")
		exit(1)
	try:
		return master_nodes[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available Master Nodes.  Please double check the names in the config files.")
		
# Classes #
class MasterNode:
	'''
		This is the class that represents and provides access to the master 
		node of a micro controller network.
		'''
	def __init__(self, serial=''):
		'''
			Constructor - opens serial port given and initializes the master node.
			'''
		global enabled
		if enabled:
			try:
				self.serial = Serial(serial)
			except Exception as e:
				log.error("Unable to open serial port %s: %s", (serial, e))
		return
	
	def send(address, msg):
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
			log.error("Messages sent to the master node must be a list of bytes!")
			return
		
		# Implementation
		message = '['
		message += '0h'
		message += address
		for x in msg:
			message += ' 0h'
			message += x
		message += ']'
		master_log.debug("Sending message: %s", message)
		self.serial.write(message+'\r')
		return
		