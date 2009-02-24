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
	dash_log.info(toStringFormat('Drive', 'Right', [speed]))
	mc.right(speed)
		
def left(speed):
	dash_log.info(toStringFormat('Drive', 'Left', [speed]))
	mc.left(speed)
		
def both(speed):
	dash_log.info(toStringFormat('Drive', 'Left', [speed]))
	dash_log.info(toStringFormat('Drive', 'Right', [speed]))
	mc.both(speed)
		
def move(speed, direction):
	dash_log.info(toStringFormat('Drive', 'Left', [speed]))
	dash_log.info(toStringFormat('Drive', 'Right', [speed]))
	mc.move(speed, direction)
	
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
	servo_micro.shutdown()
	events.shutdown()
	log.info("Robot.py has shutdown cleanly.")
	sys.exit(0)


# Initilization #
# Start logging
try:
	loggingConfigFile
	print_non_default = True
except NameError:
	loggingConfigFile = 'Configurations/logging.cfg'
	print_non_default = False
logging.config.fileConfig(loggingConfigFile)
log = logging.getLogger('Robot')
brain_log = logging.getLogger('Brain')
dash_log = logging.getLogger('Dash_Log')
dash_log.propagate = False
serverHandler = LoggingServerHandler('', PORT)
dash_log.addHandler(serverHandler)
if print_non_default:
	log.info("Using non default logging config file: %s" % loggingConfigFile)

# Start Configurations
try:
	configFile
	log.info("Using non default config file: %s" % configFile)
except NameError:
	configFile = 'Configurations/robot.cfg'
configs.init(configFile)
config = configs.get_config()

# Initialize modules #

# Sabertooth2x10
import sabertooth2x10 as saber
saber.init()
mc = saber.get_object()
# MCN
import micro_controller_network as mcn
mcn.init()
servo_micro = mcn.get_object('Servo Control')
od_micro = mcn.get_object('Obj Detection')
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


signal.signal(signal.SIGINT, shutdown_signal)

# Done
log.info("Robot.py has been loaded.")
