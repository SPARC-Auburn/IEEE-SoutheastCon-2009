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
from threading import Timer, Event
import traceback, sys

#  Setup  #
# Event
global program_stopped_event
program_stopped_event = Event()
# Global variables
global speed, direction_speed, lrf_refresh_rate, run_time, spinner_speed
speed = 0.4
direction_speed = 0.3
lrf_refresh_rate = 1
run_time = 240
spinner_speed = -0.2
# Reset all the micros
for x in micros:
	x.reset() 
sleep(1)

# Control Code #

# Handels events
def handleEvent(e, msg):
	global speed
	debug("%s - %s" % (str(e), str(msg)))	
	if e == 'On Corner':
		avoid_line()
		clear_events()
		corner_detection()
	elif e == 'LRF Detected Object':
		move(0, 0)
		r,t = msg[0]
		drive_toward_object(r, t)
	elif e == 'Micro Switch Triggered':
		info("Micro switch depressed, grabbing, stopping.")
		sort_object()
	elif e == 'Left' or e == 'Right':
		info("Desired Angle Reached")
		move(speed, 0)
	else:
		error("Unhandled event recieved, stopping.")
		stop_program()

# Refreshes the LRF monitor thread
def remonitor(t):
	global lrf_refresh_rate, program_stopped_event
	if program_stopped_event.isSet():
		return
	monitor_lrf()
	t = Timer(lrf_refresh_rate, remonitor, [t])
	t.start()

# Starts turning the robot and returns immediately, then triggers an event when the turn is complete
def turn(angle, s, d_s):
	debug("Turning %f degrees." % angle)
	move(s, d_s)
	watch_for_angle(angle)

# Starts turning the robot and doesn't return control until the turn is completed
def only_turn(angle, s, d_s):
	debug("Turning %f degrees and waiting." % angle)
	move(s, d_s)
	turn_to_angle(angle)

# Turns the robot 160 degrees to avoid leaving the boundry; clears events
def avoid_line():
	global speed, direction_speed
	only_turn(90, 0, direction_speed)
	only_turn(90, 0, direction_speed)
	move(0,0)
	zero_angle()
	move(speed,0)

# Vears toward an object at r distance, t degrees
def drive_toward_object(r,t):
	global speed, direction_speed
#	if r < 100:
#		spinner_servo.move(-1.0)
#	if r > 300:
#		spinner_servo.move(0)
	if t > 0:
		d_s = direction_speed * -1
	else:
		d_s = direction_speed
	if t != 0:
		turn(t, speed, d_s)

# Stops, checks the object, sorts it, deposits it
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
	t = Timer(5, gripper_close)
	t.start()

# Main loop
def loop():
	info("Starting Control Loop")
	global speed, lrf_refresh_rate, run_time, spinner_speed, program_stopped_event
	# Wait for the start button
	turn_start_led_on()
	wait_for_start()
	# Start the game timer
	stop_timer = Timer(run_time, stop_program)
	stop_timer.start()
	# Start the LRF refresh thread
	lrf.set_monitor_settings(angle = 60, spike = 100)
	lrf_monitor_thread = None
	lrf_monitor_thread = Timer(lrf_refresh_rate, remonitor, [lrf_monitor_thread])
	lrf_monitor_thread.start()
	gripper_open()
	spinner_servo.move(spinner_speed)
	move(speed, 0)
	corner_detection()
	while program_running():
		e, msg = get_next_event()
		handleEvent(e, msg)
	info("Exiting Control loop")
	program_stopped_event.set()

def shutdown_script():
	global program_stopped_event
	info("Preforming exit actions")
	stop_program()
	program_stopped_event.wait(5)
	if program_stopped_event.isSet():
		info("Script stopped.")
	else:
		error("Scripted timed out stopping.")


# Register the shutdown function
register_shutdown_function(shutdown_script)
# Run the loop		
loop()
