#
#  Laser.py
#  Sparc
#  
#  Created by William Woodall on 9/13/08.
#  Copyright (c) 2008 Auburn University. All rights reserved.
#


import serial

class Laser(serial.Serial):
	"""Laser class, extends serial so you can use it like a serial port.
	
	The Laser class extends the serial class so you can create it and pass it the serial port to it at the same time. i.e.:
	laser1 = laser.Laser("/dev/ttyACM0")
	You can perform any function on it that you can perform on a serial object as well as, scan()."""
	def scan(self, start = "000", finish = "768", step = "00"):
		# Check input
		