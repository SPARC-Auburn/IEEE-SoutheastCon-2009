# Echo server program
import logging, socket, types, os, string, cPickle, struct, time, re
from stat import ST_DEV, ST_INO
import logging, logging.handlers
import socket, sys
sys.path.append("../../Modules")
import events

HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port


def formatEvent(component, attribute, values): 
	n = [component, attribute] 
	n.extend([str(x) for x in values]) 
	return n 
	 	         
def toStringFormat(component, attribute, values): 
	formatList = formatEvent(component, attribute, values) 
	return ":".join(formatList) 

class Host:
	def __init__(self, host, port, wait_time = 0.05):
		self.ready = False
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.bind((host, port))
		self.sock.settimeout( wait_time )
		self.sock.listen(5)
		
	def broadcast_event(self, message):
		self.make_connection()
		#print 'Broadcasting: ', message
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
	def __init__(self, host, port, wait_time = 1):
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
		


conn, addr = range(2)
class LoggingServerHandler(logging.Handler):
    """
    A handler class which writes logging records, in pickle format, to
    a streaming socket. The socket is kept open across logging calls.
    If the peer resets it, an attempt is made to reconnect on the next call.
    The pickle which is sent is that of the LogRecord's attribute dictionary
    (__dict__), so that the receiver does not need to have the logging module
    installed in order to process the logging event.

    To unpickle the record at the receiving end into a LogRecord, use the
    makeLogRecord function.
    """

    def __init__(self, host, port, wait_time = 0.05):
        """
        Initializes the handler with a specific host address and port.

        The attribute 'closeOnError' is set to 1 - which means that if
        a socket error occurs, the socket is silently closed and then
        reopened on the next logging call.
        """
        logging.Handler.__init__(self)
        self.host = host
        self.port = port
        self.sock = None
        self.record = ''
        self.client = []
        
        self.ready = False
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.bind((host, port))
        self.sock.settimeout( wait_time )
        self.sock.listen(5)
        
    def makeConnection(self):
        #print 'Attempting to make connection'
        try:
		conn, addr = self.sock.accept()
		conn.send('Connection established: ' + HOST + ':' + PORT)
		self.client.append([conn, addr])
		#print 'New connection established. ' + len(client) + ' current connections.'
        except:
		pass
		#print 'No connection to be made'

    def send(self):
        """
        Send a pickled string to the socket.

        This function allows for partial sends which can happen when the
        network is busy.
        """
        
        self.makeConnection()
        #self.sock can be None either because we haven't reached the retry
        #time yet, or because we have reached the retry time and retried,
        #but are still unable to connect.
        if self.sock:
            #print len(self.client), ' current connections.'
		#print "Broadcast: "+record
            for thisClient in self.client:
                try:
                    thisClient[ conn ].send(str(self.record))
                except socket.error:
                    #print 'Communication Error'
                    self.client.remove(thisClient)
        #else:
            #print 'No working connections'

    def makePickle(self, record):
        """
        Pickles the record in binary format with a length prefix, and
        returns it ready for transmission across the socket.
        """
        ei = record.exc_info
        if ei:
            dummy = self.format(record) # just to get traceback text into record.exc_text
            record.exc_info = None  # to avoid Unpickleable error
        s = cPickle.dumps(record.__dict__, 1)
        if ei:
            record.exc_info = ei  # for next handler
        slen = struct.pack(">L", len(s))
        return slen + s

    def handleError(self, record):
        """
        Handle an error during logging.

        An error has occurred during logging. Most likely cause -
        connection lost. Close the socket so that we can retry on the
        next event.
        """
        #print 'handleError Function'
        if self.closeOnError and self.sock:
            self.sock.close()
            self.sock = None        #try to reconnect next time
        else:
            logging.Handler.handleError(self, record)

    def emit(self, record):
        """
        Emit a record.

        Pickles the record and writes it to the socket in binary format.
        If there is an error with the socket, silently drop the packet.
        If there was a problem with the socket, re-establishes the
        socket.
        """
	if len(self.client) > 0:
		print "Sending: "+str(record)
		self.client[0].send(str(self.record))
        self.record = record
        try:
            #s = self.makePickle(record)
            #print 'Broadcasting: ', record
            self.send()
        except (KeyboardInterrupt, SystemExit):
            raise
        except:
            self.handleError(record)

    def close(self):
        """
        Closes the socket.
        """
        #print 'Close Function'
        if self.sock:
            self.sock.close()
            self.sock = None
        logging.Handler.close(self)

# h = Host(HOST, PORT)

# while True:
	# h.countdown_TestRoutine()


# print 'Process Close'
