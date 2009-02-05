# Echo server program
import SocketClass

HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port

h = SocketClass.Host(HOST, PORT)

while True:
	h.countdown_TestRoutine()


print 'Process Close'