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
# Configs
import configs
import configobj
global config, enable
config = configs.get_config('Servo Controller')
enabled = config['enabled']
mc_name = config['micro_controller']
# Logging
import logging
log = logging.getLogger(config['logger_name'])
# micro_controller_network
import micro_controller_network
mc = micro_controller_network.get_object(mc_name)

# Static Functions #
def init():
	global config, initialized, servos
	servos = {}
	for x in config:
		if type(config[x]) is configobj.Section:
			servos[x] = Servo(config = config[x])
	initialized = True
	
def get_object(id):
	global servos, initialized
	if not initialized:
		log.critial("The servo_controller.init() method has to be called before retrieving objects.")
		exit(1)
	try:
		return servos[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available servos.  Please double check \
the names in the config files.")
		
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
		
	def dec2hex(self, dec):
		'''
			Converts a given decimal number to hex.
			'''
		hex = '%X' % dec
		if len(hex)%2 is 1: #Prevents Odd-length error with decode.
			hex = '0'+hex
		return hex.decode("hex")
				
	def move_to_position(self, position = 1500):
		'''
			Moves the servo to the given position in uSeconds.
			'''
		global enabled
		if not enabled:
			return
		log.debug("Moving to position: %i" % position)
		position = self.dec2hex(position)
		if len(position) is not 2:
			position += dec2hex(0)
		msg = []
		msg.append(self.dec2hex(int(self.pointer)))
		msg.append(position[0:1])
		msg.append(position[1:2])
		mc.send(msg)
		return
		
	def move(self, position = 0.0):
		'''
			Move the servo to a given position between 1.0 and -1.0 with 0.0 centered.
			'''
		if type(position) is float or type(position) is int:
			if position > 1.0 or position < -1.0:
				log.error("You must provide a number between 1.0 and -1.0 for the move function.")
				return
			# Do int stuff
			if position == 0.0:
				self.move_to_position(self.zero)
			elif position > 0.0:
				delta = self.max - self.zero
				pw = position * delta
				pw = pw + self.zero
				self.move_to_position(pw)
			elif position < 0.0:
				delta = self.zero - self.min
				pw = position * delta
				pw = self.zero + pw
				self.move_to_position(pw)
			else:
				return
		elif type(position) is str:
			if position is 'zero' or position is 'Zero':
				self.move_to_position(self.zero)
			elif position is 'max' or position is 'Max':
				self.move_to_position(self.max)
			elif position is 'min' or position is 'Min':
				self.move_to_position(self.min)
			else:
				log.error("When providing a string to move() it must be zero, max, or min.")
		else:
			log.error("You passed an invalid parameter to move(). Valid parameters are: an int between \
1.0 and -1.0, or one of the strings zero, max, or min.")
			return
		return
		
