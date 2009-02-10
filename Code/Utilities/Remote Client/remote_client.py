#
#  remote_client.py
#  
#
#  Created by William Woodall on 1/14/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

import os
cwd = os.getcwd()
os.chdir('../../')
import sys
sys.path.append('./Libraries')
from socket import socket, AF_INET, SOCK_DGRAM
from time import sleep, gmtime, strftime
from configobj import ConfigObj
import pygame
import pygame.event

os.chdir(cwd)

f = open('remote_client.cfg')
config = ConfigObj(f)

SERVER_IP = config['Server IP']
PORT_NUMBER = int(config['Port'])
joy_config = config['Joystick']
SENSITIVITY = float(joy_config['Sensitivity'])
SERVO_SENSITIVITY = float(joy_config['Servo_Sensitivity'])
DEAD_ZONE = float(joy_config['Dead Zone'])
THROTTLE = float(joy_config['Network Throttle'])

soc = socket( AF_INET, SOCK_DGRAM )

pygame.joystick.init()
pygame.display.init()
joy = pygame.joystick.Joystick(0)
joy.init()

def loop():
	send = True
	speed = 0.0
	direction = 0.0
	arm = 0.0
	sorter = 0.0
	gripper = 0.0
	e = None
	while True:
		try:
			e = pygame.event.wait()
			if e.axis == 1 and abs(speed - e.dict['value']) > SENSITIVITY:
				speed = e.dict['value']
				send = True
			elif e.axis == 0 and abs(direction - e.dict['value']) > SENSITIVITY:
				direction = e.dict['value']
				send = True
			elif e.axis == 4 and abs(arm - e.dict['value']) > SERVO_SENSITIVITY:
				arm = e.dict['value']
				send = True
			elif e.axis == 3 and abs(sorter - e.dict['value']) > SERVO_SENSITIVITY:
				sorter = e.dict['value']
				send = True
			elif e.axis == 2 and abs(gripper - e.dict['value']) > SERVO_SENSITIVITY:
				gripper = e.dict['value']
				send = True
			if send:
				if abs(speed) < DEAD_ZONE:
					speed = 0
				if abs(direction) < DEAD_ZONE:
					direction = 0
				if abs(arm) < DEAD_ZONE:
					arm = 0
				if abs(sorter) < DEAD_ZONE:
					sorter = 0
				move = str(speed)+" "+str(direction)+" "+str(arm)+" "+str(sorter)+" "+str(gripper)
				print move
				soc.sendto(move, (SERVER_IP, PORT_NUMBER))
				send = False
				sleep(THROTTLE)
		except AttributeError:
			pass

try:
	loop()
except KeyboardInterrupt:
	print "\nShutting down."
finally:
	soc.close()
