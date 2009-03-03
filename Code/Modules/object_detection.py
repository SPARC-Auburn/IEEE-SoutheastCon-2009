#
#  object_detection.py
#  
#
#  Created by William Woodall on 2/19/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
'''
	Module for controlling and managing object detection.
	'''

# Imports #
# Paths
import sys
sys.path.append("../Libraries")
# Configs
import configs
import configobj
global config, enable
config = configs.get_config('Obj Detection')
enabled = config['enabled']
mc_name = config['micro_controller']
# Logging
import logging
log = logging.getLogger(config['logger_name'])
# Events
import events
# Conditions
from threading import Event
# micro_controller_network
import micro_controller_network
mc = micro_controller_network.get_object(mc_name)

# Static Variables
return_codes = {'\x70':'Micro Switch Triggered',
				'\x71':'No Object',
				'\x72':'Can',
				'\x73':'Glass',
				'\x74':'Plastic',
				'\x75':'Detection Error',
		'\x76':'Start Button Pressed'}
				
command_codes = {'Check Object':'\x60',
		 'Start Button Glow':'\x61'}

# Static Functions #
def init():
	global config, initialized, obj_detection
	obj_detection = ObjDetection()
	mc.register(obj_detection)
	initialized = True
	
def get_object(id = None):
	global initialized, obj_detection
	if not initialized:
		log.critial("The object_detection.init() method has to be called before retrieving objects.")
		exit(1)
	return obj_detection
		
# Classes #
class ObjDetection:
	def __init__(self):
		self.return_codes = return_codes
		self.obj_detected = Event()
		self.start_button_event = Event()
		self.obj_detected.clear()
		self.start_button_event.clear()
		self.object = None
		self.msg = ''
		self.stop_program = True
		
	def notify(self, return_code, msg):
		# Handel the messge
		if return_code == 'Micro Switch Triggered':
			log.debug("Micro switch depressed.")
			events.triggerEvent(return_code, msg)
		elif return_code == 'Start Button Pressed':
			log.debug("Start Button depressed.")
			self.stop_program = True
			self.start_button_event.set()
		else:
			self.object = return_code
			self.msg = msg
			self.obj_detected.set()
		
	def check_obj(self):
		self.object = None
		self.obj_detected.clear()
		mc.send(command_codes['Check Object'])
		self.obj_detected.wait(5)
		if self.object is None:
			log.error("Timed out while waiting for object detection to return.")
			self.obj_detected.set()
			return
		log.debug("Object check returned: %s" % self.object)
		return self.object
		
	def wait_for_start(self):
		self.start_button_event.wait()
		self.stop_program = False

	def turn_start_led_on(self):
		mc.send(command_codes['Start Button Glow'])

	def shutdown(self):
		pass
		
		

