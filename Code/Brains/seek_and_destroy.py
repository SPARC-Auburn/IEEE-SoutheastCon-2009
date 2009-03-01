#
#  seek_and_destroy.py
#  
#
#  Created by William Woodall on 2/28/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

import brain
from robot import *
from time import sleep

# Control Code #
global stop
stop = False

def handleEvent(e, msg):
	global stop
	print e, msg
	if e == 'LRF Detected Object':
		move(0, 0)
		r, t = msg[0]
		speed = 0.2
		if t > 0:
			speed *= -1
		if t != 0:
			turn(t, speed)
		stop = True
	elif e == 'Micro Switch Triggered':
		pass
	else:
		print 'else'
		stop = True

def turn(angle, speed):
	move(0, speed)
	turn_to_angle(angle)
	move(0,0)

def loop():
	global stop
	move(.2,0)
	monitor_lrf()
	while not stop:
		e, msg = get_next_event()
		handleEvent(e, msg)
		
try:
	loop()
except Exception as e:
	pass
finally:
	shutdown()
