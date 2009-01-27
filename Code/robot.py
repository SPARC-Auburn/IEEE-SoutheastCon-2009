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

# Imports #
import sys
#print "arguments", sys.argv
# Logging
import logging
import logging.config
# Config
import configs

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
def shutdown():
	pass


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
mc = saber.get_motorcontroller()

# Done
log.info("Robot.py has been loaded.")
