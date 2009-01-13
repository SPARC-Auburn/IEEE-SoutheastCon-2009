#
#  config.py
#  
#
#  Created by William Woodall on 1/13/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

from configobj import ConfigObj
from validate import Validator

global config

def init(configFile):
	global config
	config = ConfigObj(configFile)

def get_config(sectionKey):
	global config
	return config[sectionKey]