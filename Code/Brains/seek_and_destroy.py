#
#  seek_and_destroy.py
#  
#
#  Created by William Woodall on 2/28/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

import brain
from robot import *
print "After Robot.py"
from time import sleep
from threading import Timer
import traceback, sys

#  Setup  #
for x in micros:
	x.reset() 

# Control Code #
global stop
stop = False

def handleEvent(e, msg):
	global stop
	debug("%s - %s" % (str(e), str(msg)))	
	if e == 'On Corner':
		avoid_line()
	elif e == 'LRF Detected Object':
		move(0, 0)
		sleep(0.01)
		r, t = msg[0]
		drive_toward_object(r,t)
	elif e == 'Micro Switch Triggered':
		debug("Micro switch depressed, grabbing, stopping.")
		spinner_servo.move(0)
		sort_object()
	elif e == 'Left' or e == 'Right':
		debug("Desired Angle Reached")
		move(.4,0)
		
	else:
		print 'else'
		stop = True

def remonitor(t):
	monitor_lrf()
	t = Timer(1, remonitor, [t])
	t.start()

def turn(angle, speed):
	debug("Turning to %f at %f speed." % (angle, speed))
	move(0, speed)
	watch_for_angle(angle)

def only_turn(angle, speed):
	debug("Only Turning to %f at %f speed." % (angle, speed))
	move(0, speed)
	turn_to_angle(angle)	

def avoid_line():
	only_turn(90,.3)
	only_turn(90,.3)
	move(0,0)
	zero_angle()
	move(.4,0)
	sleep(0.01)
	clear_events()
	corner_detection()

def drive_toward_object(r,t):
	speed = 0.3
	if r < 100:
		spinner_servo.move(-1.0)
	if t > 0:
		speed *= -1
	if t != 0:
		turn(t, speed)


def sort_object():
	move(0,0)
	gripper_close()
	object = check_obj()
	if object == 'Can':
		sorter_left()
	elif object == 'Plastic':
		sorter_right()
	arm_up()
	sleep(5)
	gripper_open()
	sleep(0.5)
	gripper_close()
	sleep(0.5)
	gripper_open()
	sleep(0.5)
	gripper_close()
	sleep(0.5)
	gripper_open()
	sleep(1)
	gripper_close()
	arm_down()
	sleep(1)
	sorter_center()

def psuedo_sort_object():
	gripper_close()
	move(0,0)
	arm_up()
	sleep(6)
	gripper_open()
	sleep(.5)
	gripper_close()
	sleep(.2)
	gripper_open()
	sleep(.5)
	gripper_close()
	sleep(.2)
	gripper_open()
	sleep(.5)
	gripper_close()
	arm_down()
	sleep(6)
	gripper_open()		


def stop_program():
	global stop
	stop = True

def loop():
	print "Inside Loop"
	global stop
	turn_start_led_on()
	wait_for_start()
	stop_timer = Timer(120, stop_program)
	stop_timer.start()
	t = None
	t = Timer(1, remonitor, [t])
	t.start()
	lrf.set_monitor_settings(angle = 60, spike = 100)
	gripper_open()
	move(.4,0)
	sleep(0.01)
	move(.4,0)
	monitor_lrf()

	corner_detection()	 		#call the line_follow() function	

	while not stop:
		e, msg = get_next_event()
		handleEvent(e, msg)
		
try:
	print "Before Loop"
	loop()
	print "After Loop"
except Exception as e:
	print e
	traceback.print_exc(file = sys.stdout)
finally:
	shutdown()
