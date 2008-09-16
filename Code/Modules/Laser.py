#
#  Laser.py
#  Sparc
#  
#  Created by William Woodall on 9/13/08.
#  Copyright (c) 2008 Auburn University. All rights reserved.
#


import pyserial

class laser(serial.Serial):
	def scan(self, start = "000", finish = "768", step = "00"):
		