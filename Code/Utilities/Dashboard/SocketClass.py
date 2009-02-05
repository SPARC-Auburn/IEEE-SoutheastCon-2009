# Echo server program
import socket
import time

HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port

class Host:
	def __init__(self, host, port, wait_time = 0.05):
		self.ready = False
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.bind((host, port))
		self.sock.settimeout( wait_time )
		self.sock.listen(5)
		
	def broadcast_event(self, message):
		self.make_connection()
		print 'Broadcasting: ', message
		try:
			self.conn.send(message)
		except:
			self.ready = False
			
	def make_connection(self):
		if not self.ready:
			try:
				self.conn, self.addr = self.sock.accept()
				self.ready = True
			except:
				print 'No connection to be made'
				
	def countdown_TestRoutine(self):
		self.broadcast_event('Process Started >>')
		time.sleep(1)
		self.broadcast_event('Begin countdown:')
		time.sleep(1)
		for c in range(10, 0, -1):
			self.broadcast_event(str(c))
			time.sleep(1)
			
			
class Client:
	def __init__(self, host, port, wait_time = 0.05):
		self.ready = False
		self.message = ''
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.settimeout(wait_time)
		self.sock.connect((host, port))
		
	def get_message(self):
		try:
			message = self.sock.recv(1024)
			# print message
			return message
		except:
			# print 'No message recieved'
			return ''
		


# h = Host(HOST, PORT)

# while True:
	# h.countdown_TestRoutine()


# print 'Process Close'