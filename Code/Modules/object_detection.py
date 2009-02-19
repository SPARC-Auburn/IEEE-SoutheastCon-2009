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
# Queue
import Queue
# Logging
import logging
log = logging.getLogger('Obj Detection')
# Configs
import configs
import configobj
global config, enable
config = configs.get_config('Obj Detection')
enabled = config['enabled']
mc_name = config['micro_controller']
# Events
import events
# Conditions
from threading import Event
# micro_controller_network
import micro_controller_network

# Static Variables
return_codes = {0x70:'Micro Switch Triggered',
				0x71:'No Object',
				0x72:'Can',
				0x73:'Glass',
				0x74:'Plastic',
				0x75:'Detection Error'}

# Static Functions #
def init():
	global config, initialized, obj_detection, mc
	mc = micro_controller_network.get_object(mc_name)
	obj_detection = ObjDetection()
	mc.register(obj_detection)
	initialized = True
	
def get_object(id):
	global initialized, obj_detection
	if not initialized:
		log.critial("The object_detection.init() method has to be called before retrieving objects.")
		exit(1)
	return obj_detection
		
# Classes #
class ObjDetection:
	def __init__(self):
		self.return_codes = return_codes
		self.queue = Queue.Queue(1024)
		self.obj_detected = Event()
		self.obj_detected.clear()
		self.object = None
		
	def notify(self, return_code, msg):
		# Handel the messge
		if return_code == 'Micro Switch Triggered':
			events.queue.put(return_code)
		else:
			self.object = return_code
			self.obj_detected.set()
		
	def check_obj(self):
		object = None
		self.obj_detected.clear()
		mc.send(0x60)
		self.obj_detected.wait(5)
		if object is None:
			log.error("Timed out while waiting for object detection to return.")
			return
		return object
		
		