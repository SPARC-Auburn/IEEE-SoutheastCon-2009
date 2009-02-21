#
#  sabertooth2x10.py
#  
#  This is a module for controlling the Sabertooth 2x10 DC Motor controller.
#  Datasheet: http://www.dimensionengineering.com/datasheets/Sabertooth2x10.pdf
#  FTDS:
#    Because Sabertooth controls two motors with one 8 byte character, when operating in Simplified 
#    Serial mode, each motor has 7 bits of resolution. Sending a character between 1 and 127 will 
#    control motor 1. 1 is full reverse, 64 is stop and 127  is full forward. Sending a character between 
#    128 and 255 will control motor 2. 128 is full reverse, 192 is stop and 255 is full forward. 
#    Character 0 (hex 0x00) is a special case. Sending this character will shut down both motors.
#  
#  Created by William Woodall on 12/31/08.
#  Copyright (c) 2008 Auburn University. All rights reserved.
#

'''
	Module for controlling a sabertooth2x10 motor controller.
	TODO: 
	* Documentation...
	'''

# Imports #
# Configs
import configs
global config, enabled
config = configs.get_config('Sabertooth2x10')
enabled = config['enabled']
# Logging
import logging
log = logging.getLogger(config['logger_name'])
# PySerial
if enabled:
	try:
		from serial import Serial
	except Exception as e:
		log.error("Serial cannot be imported, you may need to install it: %s" % e)
else:
	log.warning("The Motor Controller is disabled!  This means the motors will quietly do nothing!")

# Static Functions #
def init():
	global config, initialized, motorcontrollers
	motorcontrollers = {}
	# Implement dynamic number of mc's using the config files maybe?
	motorcontrollers['motor controller'] = MotorController(config['serial_port'])
	initialized = True

def get_object(id = 'motor controller'):
	global motorcontrollers, initialized
	if not initialized:
		log.critical("The sabertooth2x10.init() method has to be called before retrieving objects.")
	try:
		return motorcontrollers[id]
	except KeyError:
		log.error("You provided an invalid id for any of the available Motor Controllers.  Please double check the names in the config files.")

# Classes #
class MotorController:
	'''
		This is the class that contains all data and functions 
		related to controlling the Sabertooth 2x10 motor controllers.
		'''
	def __init__(self, serial=''):
		'''
			Constructor - 'serial' is the unix serial address that the
			controller is connected to.
			'''
		global enabled
		if enabled:
			try:
				self.serial = Serial(serial)
			except Exception as e:
				log.error("Unable to connect to the motor controller on port %s.  The motor controller will not function.  See error bellow." % serial)
				log.error("%s" % e)
		self.swap_motors = bool(config['swap_motors'])
	
	def dec2hex(self, dec):
		'''
			Converts a given decimal number to hex.
			'''
		hex = '%X' % dec
		if len(hex) < 2: #Prevents Odd-length error with decode.
			hex = '0'+hex
		return hex.decode("hex")
	
	def right(self, speed=1.0):
		'''
			Drives the right motor at a given speed between -1.0 and 1.0.
			'''
		global enabled
		if not enabled:
			return
		if speed < -1.0 or speed > 1.0:
			log.error("Must provide speed between -1.0 and 1.0")
			return
		speed = int(speed * 64) + 64
		#Make sure that the speed is not 0.
		if speed is 0:
			speed = 1
		#Check to see if the motors are swapped, one needs to be 
		#offset 127 and the other needs to not go to 128.
		if self.swap_motors:
			speed = speed + 127
		elif speed is 128:
			speed = 127
		#Convert the speed to hex
		speed = self.dec2hex(speed)
		#Send the command
		self.serial.write(speed)
	
	def left(self, speed=1.0):
		'''
			Drives the left motor at a given speed between -1.0 and 1.0.
			'''
		global enabled
		if not enabled:
			return
		if speed < -1.0 or speed > 1.0:
			log.error("Must provide speed between -1.0 and 1.0")
			return
		speed = int(speed * 64) + 64
		#Make sure that the speed is not 0.
		if speed is 0:
			speed = 1
		#Check to see if the motors are swapped.
		if not self.swap_motors:
			speed = speed + 127
		elif speed is 128:
			speed = 127
		#Convert the speed to hex
		speed = self.dec2hex(speed)
		#Send the command
		self.serial.write(speed)
	
	def both(self, speed=1.0):
		'''
			Drives both motors at a given speed between -1.0 and 1.0.
			This is done by sending the right motor a command then the left
			except for when you give 0 or 0.0, which will send a single
			command to stop both the motors at the same time.
			'''
		global enabled
		if not enabled:
			return
		#If speed is 0 then send the stop all command
		if speed is 0:
			self.serial.write('\x00')
			return
		self.drive_right(speed)
		self.drive_left(speed)
	
	def move(self, speed=1.0, direction=0.0):
		'''
			Drives both motors at a given speed <-1.0 ... 1.0> and
			augments the speed to each motor based on direction
			<-1.0 ... 1.0>.  Direction 0 is straght ahead, -1.0 is
			hard left and 1.0 is hard right.  Giving speed (x > 0)
			and direction (r = -1.0 or 1.0) the robot should spin in
			place left or right respectively.
			'''
		global enabled
		if not enabled:
			return
		#Validate the parameters
		if speed < -1.0 or speed > 1.0:
			log.error("Speed given to the move() function must be between -1.0 and 1.0 inclusively.")
			return
		if direction < -1.0 or direction > 1.0:
			log.error("Direction given to the move() function must be between -1.0 and 1.0 inclusively.")
			return
		#First calculate the speed of each motor then send the commands
		#Account for speed
		leftSpeed = speed
		rightSpeed = speed
		#Account for direction
		leftSpeed = leftSpeed + direction
		rightSpeed = rightSpeed - direction
		#Account for going over 1.0 or under -1.0
		if leftSpeed < -1.0:
			leftSpeed = -1.0
		if leftSpeed > 1.0:
			leftSpeed = 1.0
		if rightSpeed < -1.0:
			rightSpeed = -1.0
		if rightSpeed > 1.0:
			rightSpeed = 1.0
		#Send the commands
		self.left(leftSpeed)
		self.right(rightSpeed)
		return
		