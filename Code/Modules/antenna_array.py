#
#  antenna_array.py
#  
#
#  Created by William Woodall on 2/20/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

# Imports #
# Paths
import sys
sys.path.append("../Libraries")
# Configs
import configs
import configobj
global config
config = configs.get_config('Antenna Array')
enabled = config['enabled']
mc_name = config['micro_controller']
# Logging
import logging
log = logging.getLogger(config['logger_name'])
# Events
import events
# Event
from threading import Event
# Sleep
from time import sleep
# micro_controller_network
import micro_controller_network
mc = micro_controller_network.get_object(mc_name)

# Static Variables
return_codes = {'\x41':'Left',
				'\x42':'Right',
				'\x43':'Dead On',
				'\x49':'No Line',
				'\x44':'On Corner',
				'\x46':'Left Back',
				'\x47':'Right Back',
				'\x48':'Right Front'}
				
command_codes = {'Line Follow':'\x41',
				 'Corner Detection':'\x42',
				 'Line Detection':'\x43'}

# Static Functions #
def init():
	global config, initialized, ant_array
	ant_array = AntennaArray()
	mc.register(ant_array)
	initialized = True
	
def get_object(id = None):
	global initialized, ant_array
	if not initialized:
		log.critial("The object_detection.init() method has to be called before retrieving objects.")
		exit(1)
	return ant_array
		
# Classes #
class AntennaArray:
	def __init__(self):
		self.return_codes = return_codes
		
	def notify(self, return_code, msg):
		# Handel the messge
		if return_code == 'Left':
			log.debug("Antenna array indicates we are LEFT of the line.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'Right':
			log.debug("Antenna array indicates we are RIGHT of the line.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'Dead On':
			log.debug("Antenna array indicates we are DEAD ON the line.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'No Line':
			log.debug("Antenna array indicates NO LINE is detected.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'On Corner':
			log.debug("Antenna array indicates we are ON the CORNER.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'Left Back':
			log.debug("Antenna array indicates the line is near the LEFT BACK of the robot.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'Right Back':
			log.debug("Antenna array indicates the line is near the RIGHT BACK of the robot.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'Right Front':
			log.debug("Antenna array indicates the line is near the RIGHT FRONT of the robot.")
			events.triggerEvent(return_code, msg)
			
	def shutdown(self):
		pass
		
	def cal_ant(self):
		mc.send(command_codes['Calibrate Antenna'])
		sleep(1)
		
	def line_follow(self, enable = True):
		if enable:
			mc.send(command_codes['Line Follow']+'1')
		else:
			mc.send(command_codes['Line Follow']+'0')
			
	def corner_detection(self, enable = True):
		if enable:
			mc.send(command_codes['Corner Detection']+'1')
		else:
			mc.send(command_codes['Corner Detection']+'0')
		
	def line_detection(self, enable = True):
		if enable:
			mc.send(command_codes['Line Detection']+'1')
		else:
			mc.send(command_codes['Line Detection']+'0')