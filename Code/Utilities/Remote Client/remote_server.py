#
#  remote_server.py
#  
#
#  Created by William Woodall on 1/14/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
import os
os.chdir('../../')
import sys
sys.path.append('./')

from robot import *

from socket import socket, gethostbyname, AF_INET, SOCK_DGRAM

PORT_NUMBER = 5000

hostName = ''

soc = socket(AF_INET, SOCK_DGRAM)
soc.bind( (hostName, PORT_NUMBER) )

info("Server started on port: %i" % PORT_NUMBER)

# Loop
def loop():
	while True:
		(data, addr) = soc.recvfrom( 1024 )
		debug("Recieved Pack from %s: %s" % (addr, data))
		p = data.partition(' ')
		speed = float(p[0])
		p = p[2].partition(' ')
		direction = float(p[0])
		p = p[2].partition(' ')
		arm = float(p[0])
		p = p[2].partition(' ')
		sorter = float(p[0])
		p = p[2].partition(' ')
		gripper = float(p[0])
		move(speed, direction)
		arm_servo.move(arm)
		sorter_servo.move(sorter)
		gripper_servo.move(gripper)
		
try:
	loop()
except KeyboardInterrupt:
	print
	info("Shutting Down.")
finally:
	shutdown()
	soc.close()
