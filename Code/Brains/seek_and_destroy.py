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

# Control Code #

def handleEvent(e, msg):
	if e == 'LRF Object Detected':
		move(0, 0)
		sys.exit(0)
	elif e == 'Micro Switch Triggered':
		print 'asdf'

def loop():
	while True:
		e, msg = get_next_event()
		handleEvent(e, msg)
		
if __name__ == '__main__':
	try:
		loop()
	except Exception as e:
		pass
	finally:
		shutdown()
		sys.exit()