#
#  servo_controller.py
#  
#
#  Created by William Woodall on 1/29/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
'''
	Module for communicating with the servo controller.
	'''

# Imports #
# Paths
import sys
sys.path.append("../Libraries")
# Logging
import logging
log = logging.getLogger('Servo Ctrl')
# Configs
import configs
import configobj
global config, enable
config = configs.get_config('Servo Controller')
enabled = config['enabled']
# micro_controller_network
import micro_controller_network

# Static Functions #
def init():
	global config, initialized, servos, address, mcn
	servos = {}
	for x in config:
		print x
		if type(config[x]) is configobj.Section:
			servos[x] = Servo(config = config[x])
	address = config['address']
	mcn = micro_controller_network.get_object()
	initialized = True
	
def get_object(id):
	global servos, initialized
	if not initialized:
		log.critial("The servo_controller.init() method has to be called before retrieving objects.")
		exit(1)
	try:
		return servos[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available servos.  Please double check the names in the config files.")
		
# Classes #
class Servo:
	'''
		This class represents and allows control of a servo controlled by 
		a servo controller on a micro controller network.
		'''
	def __init__(self, pointer = None, zero = 1500, max = 2400, min = 600, config = None):
		'''
			Constructor - Creates a servo obejct given a configuration list.
			'''
		global config
		if config is None and pointer is None:
			log.error("When creating a servo either pointer or config must be defined, but not both.")
		if config is not None and pointer is not None:
			log.error("When creating a servo either pointer or config must be defined, but not both.")
		if config is None:
			self.pointer = str(pointer)
			self.zero = zero
			self.max = max
			self.min = min
		else:
			self.pointer = config['pointer']
			self.zero = int(config['zero'])
			self.max = int(config['max'])
			self.min = int(config['min'])
		return
				
	def move_to_position(position = 1500):
		'''
			Moves the servo to the given position.
			'''
		global enabled, mcn, address
		if not enabled:
			return
		position = '%X' % position
		if len(position) % 2 is 1:
			position = '0' + position
		msg = []
		if len(pointer) <= 1:
			pointer = '0' + pointer
		msg.append(pointer)
		for x in range(len(position)):
			if x % 2 is 1:
				msg.append(position[x:x+2])
		mcn.send(address, msg)
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		a