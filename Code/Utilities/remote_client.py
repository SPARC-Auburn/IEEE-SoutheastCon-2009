#
#  remote_client.py
#  
#
#  Created by William Woodall on 1/14/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#

from socket import socket, AF_INET, SOCK_DGRAM
from time import sleep, gmtime, strftime

SERVER_IP = '127.0.0.1'
PORT_NUMBER = 5000

soc = socket( AF_INET, SOCK_DGRAM )

def loop():
	while True:
		localTime = strftime("%H:%M:%S", gmtime())
		soc.sendto("packet_time = " + localTime, (SERVER_IP, PORT_NUMBER))
		sleep(0.1)
		
try:
	loop()
except KeyboardInterrupt:
	print "\nShutting down."
finally:
	soc.close()