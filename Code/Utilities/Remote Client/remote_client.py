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
INVERT_X = joy_config['Invert_X']
if INVERT_X == 'True':
	INVERT_X = True
else:
	INVERT_X = False
INVERT_Y = joy_config['Invert_Y']
if INVERT_Y == 'True':
	INVERT_Y = True
else:
	INVERT_Y = False
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
	arm = -1.0
	sorter = 0.0
	gripper = 1.0
	e = None
	while True:
		try:
			e = pygame.event.wait()
			# If speed
			if e.axis == 1 and abs(speed - e.dict['value']) > SENSITIVITY:
				speed = e.dict['value']
				if INVERT_Y:
					speed *= -1
				send = True
			# Else if direction
			elif e.axis == 0 and abs(direction - e.dict['value']) > SENSITIVITY:
				direction = e.dict['value']
				if INVERT_X:
					direction *= -1
				send = True
			# Else if arm
			elif e.axis == 4:
				if abs(e.dict['value']) < SENSITIVITY:
					pass
				elif (1 - abs(e.dict['value'])) < SENSITIVITY and abs(e.dict['value']) - 1 < SENSITIVITY:
					if e.dict['value'] > 0:
						if arm is not -1.0:
							send = True
						arm = -1.0
					else:
						if arm is not 1.0:
							send = True
						arm = 1.0
			# Else if sorter
			elif e.axis == 3:
				# If it is close to zero
				if abs(e.dict['value']) < SENSITIVITY:
					if sorter is not 0:
						send = True
						sorter = 0
				# If it is close to 1.0 or -1.0
				elif (1 - abs(e.dict['value'])) < SENSITIVITY:
					# If it was positive
					if e.dict['value'] > 0:
						if sorter is not 1.0:
							sorter = 1.0
							send = True
					# If it was negative
					elif e.dict['value'] < 0:
						if sorter is not -1.0:
							sorter = -1.0
							send = True
			# Else if gripper
			elif e.axis == 2:
				if abs(e.dict['value']) < SENSITIVITY:
					pass
				elif abs(e.dict['value']) - 1 < SENSITIVITY:
					if e.dict['value'] > 0:
						if gripper is not 1.0:
							send = True
						gripper = 1.0
					else:
						if gripper is not -1.0:
							send = True
						gripper = -1.0
			# If new information
			if send:
				if abs(speed) < DEAD_ZONE:
					speed = 0
				if abs(direction) < DEAD_ZONE:
					direction = 0
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
