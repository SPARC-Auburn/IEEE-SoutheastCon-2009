#
#  angular_accelerometer.py
#  
#
#  Created by William Woodall on 2/21/09.
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
config = configs.get_config('Angular Accelerometer')
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
return_codes = {'\x60':'Angle: ',
				'\x61':'Left',
				'\x62':'Right'}
				
command_codes = {'Get Angle':'\x60',
				 'Monitor Angle':'\x61',
				 'Zero Angle':'\x62'}

# Static Functions #
def init():
	global config, initialized, angular_accel
	angular_accel = AngularAccelerometer()
	mc.register(angular_accel)
	initialized = True
	
def get_object(id = None):
	global initialized, angular_accel
	if not initialized:
		log.critial("The angular_accelerometer.init() method has to be called before retrieving objects.")
		exit(1)
	return angular_accel
		
# Classes #
class AngularAccelerometer:
	def __init__(self):
		self.return_codes = return_codes
		self.angle_got = Event()
		self.angle_got.clear()
		self.angle_reached = Event()
		self.angle_reached.clear()
		self.angle = 0
		self.direction = None
		self.passive = False
		
	def notify(self, return_code, msg):
		# Handel the messge
		if return_code == 'Angle: ':
			# Angle returned
			log.debug("Current Angle: %s", msg[2:])
			self.angle = int(msg[2:])
			self.angle_got.set()
		elif return_code == 'Left':
			log.debug("Angle change completed in the LEFT direction.")
			if not self.passive:
				self.direction = 'Left'
				self.angle_reached.set()
			else:
				events.triggerEvent(return_code, msg)
		elif return_code == 'Right':
			log.debug("Angle change completed in the RIGHT direction.")
			if not self.passive:
				self.direction = 'Right'
				self.angle_reached.set()
			else:
				events.triggerEvent(return_code, msg)
		else:
			log.error("Error in handeling notifications.")
			
	def shutdown(self):
		pass
		
	def dec2hex(self, dec):
		'''
			Converts a given decimal number to hex.
			'''
		hex = '%X' % dec
		if len(hex) < 2: #Prevents Odd-length error with decode.
			hex = '0'+hex
		return hex.decode("hex")
			
	def get_angle(self):
		self.angle = 0
		self.angle_got.clear()
		mc.send(command_codes['Get Angle'])
		self.angle_got.wait(5)
		if not self.angle_got.isSet():
			log.error("Get Angle timeout while waiting for a response from the angular rate sensor.")
			return
		else:
			return self.angle
			
	def monitor_angle(self, delta):
		self.passive = False
		self.direction = None
		self.angle_reached.clear()
		d = self.dec2hex(delta)
		mc.send(command_codes['Monitor Angle']+d)
		while not self.angle_reached.isSet():
			self.angle_reached.wait(1)
		# Angle returned
		self.angle = self.get_angle()
		return self.angle
		
	def monitor_angle_passive(self, delta):
		self.passive = True
		self.direction = None
		d = self.dec2hex(delta)
		mc.send(command_codes['Monitor Angle']+d)
		return
		
	def zero_angle(self):
		mc.send(command_codes['Zero Angle'])
