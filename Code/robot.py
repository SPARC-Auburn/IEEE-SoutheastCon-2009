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
import config

# Modules
# Sabertooth2x10 module
import sabertooth2x10 as saber

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

def debug(msg):
	brain_log.debug(msg)

# Start logging
try:
	loggingConfigFile
	print_non_default = True
except NameError:
	loggingConfigFile = 'Configurations/logging.cfg'
	print_non_default = False
logging.config.fileConfig(loggingConfigFile)
log = logging.getLogger('Robot')
if print_non_default:
	log.info("Using non default logging config file: %s" % loggingConfigFile)

# Start Configurations
try:
	configFile
	log.info("Using non default config file: %s" % configFile)
except NameError:
	configFile = 'Configurations/robot.cfg'
config.init(configFile)

# Initialize modules #
# Sabertooth2x10
global mc
mc = saber.MotorController()


# Done
log.info("Robot.py has been loaded.")