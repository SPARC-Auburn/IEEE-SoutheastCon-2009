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
from threading import Timer
import traceback, sys
# Control Code #
global stop
stop = False

def handleEvent(e, msg):
	global stop
	debug("%s - %s" % (str(e), str(msg)))
	if e == 'LRF Detected Object':
		move(0, 0)
		sleep(0.01)
		r, t = msg[0]
		speed = 0.3
		if r < 200:
			spinner_servo.move(-1.0)
		if t > 0:
			speed *= -1
		if t != 0:
			turn(t, speed)
		sleep(0.01)
		move(.4,0)
	elif e == 'Micro Switch Triggered':
		debug("Micro switch depressed, grabbing, stopping.")
		spinner_servo.move(0)
		gripper_close()
		stop = True
	else:
		print 'else'
		stop = True
	clear_events()

def remonitor(t):
	monitor_lrf()
	t = Timer(1, remonitor, [t])
	t.start()
t = None
t = Timer(1, remonitor, [t])
t.start()

def turn(angle, speed):
	debug("Turning to %f at %f speed." % (angle, speed))
	move(0, speed)
	turn_to_angle(angle)
	move(0,0)

def loop():
	global stop
	lrf.set_monitor_settings(angle = 60, spike = 100)
	gripper_open()
	move(.4,0)
	sleep(0.01)
	move(.4,0)
	monitor_lrf()
	while not stop:
		e, msg = get_next_event()
		handleEvent(e, msg)
		
try:
	loop()
except Exception as e:
	print e
	traceback.print_exc(file = sys.stdout)
finally:
	gripper_open()
	shutdown()
