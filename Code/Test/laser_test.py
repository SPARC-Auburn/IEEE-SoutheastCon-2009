# ! /usr/bin/python

# Add the relative Module folder to the path.
import sys
sys.path.append("../Modules/")

import time
import os
from logging import *
import ConfigParser
import laser

def logger_init():
	global logger
	# Create logger
	logger = getLogger("laser_test")
	logger.setLevel(DEBUG)
	# Create Stream Handler
	ch = StreamHandler()
	ch.setLevel(DEBUG)
	# Create formatter
	formatter = Formatter("%(asctime)s>%(levelname)s: %(message)s")
	# Add recursively
	ch.setFormatter(formatter)
	logger.addHandler(ch)
	

def cfg_init():
	global logger, cfg
	"""Initializes the configurations by either opening \
	an existing config file or by creating one by from defaults.
	
	The configuration file is called laser_test.conf and can be edited """
	cfg = ConfigParser.ConfigParser()
	if os.path.isfile('laser_test.conf'):
		logger.debug("Using existing laser_test.conf file.")
		cfg.read('laser_test.conf')
	else:
		# Generate default config file.
		logger.info("Generating laser_test.conf from defaults.")
		cfg.add_section('Laser Range Finder')
		cfg.set('Laser Range Finder', 'port', "/dev/ttyACM0")
		cfg.set('Laser Range Finder', 'baud', 115200)
		cfg.write(open('laser_test.conf', "w"))

# Initing logger
logger_init()

# Initing configurations
cfg_init()

lrf = laser.Laser(cfg.get('Laser Range Finder', 'port'))
print lrf.scan()

