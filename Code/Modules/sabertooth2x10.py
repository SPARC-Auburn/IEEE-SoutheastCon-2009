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

'''Module for controlling a sabertooth2x10 motor controller.'''

# Imports #
# PySerial
from serial import Serial


class MotorController:
	'''This is the class that contains all data
and functions related to controlling the Sabertooth 2x10 motor controllers.'''
	def __init__(self, serial="/dev/ttyS0"):
		'''Constructor - 'serial' is the unix serial address that the controller is connected to.'''
		self.sio = Serial(serial)
		self.swap_motors = False
		
	def move(self, speed=1.0, direction=0.0):
		pass
	
	def dec2hex(self, dec):
		return ('%X' % dec).decode("hex")
	
	def drive(self, motor="both", speed=1.0):
		'''Drives the motors.  'motor' can be Left, Right, or Both.  'speed' can be between -1.0 and 0.0 inclusively.'''
		#Make sure that the speed is range and that the motor command is valid
		if speed < -1.0 or speed > 1.0:
			print "Must provide speed between -1.0 and 1.0" ### USE LOGGING HERE
		if motor != 'both' or motor != 'left' or motor != 'right':
			print "Invalid motor option. Use: both or left or right" ### USE LOGGING HERE
		
		#Take care of the special 0 speed case
		if speed is 0.0 and motor is 'both':
			speed = '\x00'
		#Figure out the speed command, then execute to the correct motor.
		else:
			speed = int(speed * 64) + 65
		
		#Convert the dec to Hex
		if speed is not '\x00':
			speedr = self.dec2hex(speed)
			#Prevent roll over
			if speed is 127:
				speed = speed - 1
			speedl = self.dec2hex(speed + 127)
			
		#If the swap motors flag is on swap values
		if self.swap_motors:
			tmp = speedl
			speedl = speedr
			speedr = tmp
		
		#Send the command to the motor(s)
		if motor is 'both':
			self.sio.write(speedl)
			self.sio.write(speedr)
		elif motor is 'left':
			self.sio.write(speedl)
		else:
			self.sio.write(speedr)
		
