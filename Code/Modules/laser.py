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
        """This function will scan from start to stop, returning a reading for every step number of scans.

        This function allows for specifying start, stop, and step values,
        but also provides default values, so all parameters are optional.
        This function will return an array with a length equal to the number scans,
        with each element in the array containing the corresponding reading.
        i.e.: readings[i] is equal to reading of the ith+1 scan."""
		# Check input
		pass

    def clear():
        """This function clears all input/output buffers from the serial device."""
        pass
