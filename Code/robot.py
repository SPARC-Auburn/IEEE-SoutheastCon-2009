#!/usr/bin/python -i 

#
#  robot.py
#  
#
#  Created by William Woodall on 12/4/08.
#  Copyright (c) 2008 Auburn University. All rights reserved.
#

# Paths #
import sys
sys.path.append("Libraries")
sys.path.append("Modules")
sys.path.append("Utilities/Dashboard")


# Imports #
# Constants
from Constants import *
# Dashboard communication
from Networking import *
# Signal
import signal
# Time
from time import *
#print "arguments", sys.argv
# Logging
import logging
import logging.config
# Config
import configs
# Events
import events

#serverHandler = LoggingServerHandler('', PORT)

# Functions #
# These functions should be mostly 'pass through' functions.
# Which basically means that it allows functions to be called
# directly as opposed to library.function in the brain.py.
# This will be the case so long as you do:
# from robot import *
# or
# python -i robot.py

# Motor Controller Related Functions
def right(speed):
	mc.right(speed)
		
def left(speed):
	mc.left(speed)
		
def both(speed):
	mc.both(speed)
		
def move(speed, direction):
	mc.move(speed, direction)
	
# Laser Range Finder Related Functions
def scan():
	return lrf.scan()
	
def clear_lrf():
	lrf.clear()
	
# Servo Related Functions
def arm_up():
	dash_log.info(toStringFormat('Arm', 'Angle', [185.0]))
	arm_servo.move(-1.0)
	
def arm_down():
	dash_log.info(toStringFormat('Arm', 'Angle', [0.0]))
	arm_servo.move(1.0)
	
def gripper_open():
	dash_log.info(toStringFormat('Gripper', 'Status', [Open]))
	gripper_servo.move(-1.0)
	
def gripper_close():
	dash_log.info(toStringFormat('Gripper', 'Status', [Closed]))
	gripper_servo.move(1.0)
	
def sorter_left():
	dash_log.info(toStringFormat('Sorter', 'Position', [0]))
	sorter_servo.move(-1.0)

def sorter_right():
	dash_log.info(toStringFormat('Sorter', 'Position', [2]))
	sorter_servo.move(1.0)
	
def sorter_center():
	dash_log.info(toStringFormat('Sorter', 'Position', [1]))
	sorter_servo.move(0)
	
def spinner_start():
	spinner_servo.move(-1.0)

def spinner_stop():
	spinner_servo.move(0.0)

# Object Detection
def check_obj():
	obj_detection.check_obj()

# Antenna Array
def line_follow():
	ant_array.line_follow()

def corner_detection():
	ant_array.corner_detection()

def line_detection():
	ant_array.line_detection()
	
# Events
def wait_for_event():
        events.wait_for_event()
       
def get_last_event():
        return events.get_last_event()

def get_next_event():
        wait_for_event()
        return get_last_event()
       
def clear_events():
	events.clear_events()
	return
	   
def list_possible_events():
        for micro in micros:
                print micro.name+":"
                for service in micro.services:
                        print "\t"+service.name+":"
                        for rc in service.return_codes:
                                print "\t\t"+service.return_codes[rc]

# Logging Related Functions
def debug(msg):
	brain_log.debug(msg)

def info(msg):
	brain_log.info(msg)

def warning(msg):
	brain_log.warning(msg)
	
def error(msg):
	brain_log.error(msg)
	
def critical(msg):
	brain_log.critical(msg)
	
# Misc Functions
def shutdown_signal(signal, frame):
	shutdown()
	
def shutdown():
	mc.shutdown()
	lrf.shutdown()
	for micro in micros:
		micro.shutdown()
	log.info("Robot.py has shutdown cleanly.")
	sys.exit(0)

# Initilization #
# Start logging
try:
	loggingConfigFile = open('Configurations/logging.cfg')
except IOError:
	try:
		loggingConfigFile = open('Configurations/logging.cfg.default')
	except IOError:
		print ">>>>ERROR: No logging config file could be opened.<<<<"
logging.config.fileConfig(loggingConfigFile)
log = logging.getLogger('Robot')
brain_log = logging.getLogger('Brain')
dash_log = logging.getLogger('Dash_Log')
dash_log.propagate = False
serverHandler = LoggingServerHandler('', PORT)
dash_log.addHandler(serverHandler)
log.debug("Using logging config file %s" % loggingConfigFile.name)

# Start Configurations
try:
	configFile = open('Configurations/robot.cfg')
except IOError:
	try:
		configFile = open('Configurations/robot.cfg.default')
	except:
		log.error("No config file could be opened.")
configs.init(configFile)
config = configs.get_config()
log.debug("Using config file %s" % configFile.name)

# Initialize modules #

# Sabertooth2x10
import sabertooth2x10 as saber
saber.init()
mc = saber.get_object()
# Laser Range Finder
import laser
laser.init()
lrf = laser.get_object()
lrf.clear()
# MCN
import micro_controller_network as mcn
mcn.init()
micros = []
servo_micro = mcn.get_object('Servo Control')
micros.append(servo_micro)
od_micro = mcn.get_object('Obj Detection')
micros.append(od_micro)
array_micro = mcn.get_object('Array Micro')
micros.append(array_micro)
# Servo Controller
import servo_controller as sc
sc.init()
arm_servo = sc.get_object('Arm Servo')
arm_down()
sorter_servo = sc.get_object('Sorter Servo')
sorter_center()
gripper_servo = sc.get_object('Gripper Servo')
gripper_close()
spinner_servo = sc.get_object('Spinner Servo')
spinner_servo.move(-1.0)
# Object Detection
import object_detection
object_detection.init()
obj_detection = object_detection.get_object()
# Antenna Array
import antenna_array
antenna_array.init()
ant_array = antenna_array.get_object()
# Angular Rate


signal.signal(signal.SIGINT, shutdown_signal)

# Done
log.info("Robot.py has been loaded.")
