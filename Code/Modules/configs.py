#
#  config.py
#  
#
#  Created by William Woodall on 1/13/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

import sys
sys.path.append('../Libraries')

from configobj import ConfigObj
from validate import Validator

global config

def convert_bools(s, k):
	if s[k] == 'True' or s[k] == 'true' or s[k] == '1':
		s[k] = True
	if s[k] == 'False' or s[k] == 'false' or s[k] == '0':
		s[k] = False
	
def init(configFile):
	global config
	config = ConfigObj(configFile)
	config.walk(convert_bools)
	
def get_config(sectionKey = ''):
	global config
	if sectionKey is '':
		return config
	else:
		return config[sectionKey]