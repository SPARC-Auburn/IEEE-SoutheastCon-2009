import logging, logging.handlers
import time
import Networking

HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port

def formatEvent(component, attribute, values):
    n = [component, attribute]
    
    n.extend([str(x) for x in values])
    return n
	
def toStringFormat(component, attribute, values):
	formatList = formatEvent(component, attribute, values)
	return ":".join(formatList)

rootLogger = logging.getLogger('')
rootLogger.setLevel(logging.DEBUG)
socketHandler = logging.handlers.SocketHandler('localhost',
                    logging.handlers.DEFAULT_TCP_LOGGING_PORT)
hdlrConsole = logging.StreamHandler('/dev/stdout')
# # don't bother with a formatter, since a socket handler sends the event as
# # an unformatted pickle
serverHandler = Networking.LoggingServerHandler('127.0.0.1', PORT)
# format = logging.Formatter("%(asctime)s - %(component)s - %(attribute)s - %(value)s")
# serverHandler.setFormatter(format)
rootLogger.addHandler(serverHandler)
rootLogger.addHandler(socketHandler)
# rootLogger.addHandler(hdlrConsole)


# Now, we can log to the root logger, or any other logger. First the root...
logging.info('Jackdaws love my big sphinx of quartz.')

# Now, define a couple of other loggers which might represent areas in your
# application:

logger1 = logging.getLogger('myapp.area1')
logger2 = logging.getLogger('myapp.area2')

# state = ['Motor', 'Left', [5]]
# list = toStringList('Motor', 'Left', [5,6,7])
# print 'List = ', list

while True:
    # logger1.debug(extra=d)
    logger1.info(toStringFormat('Drive', 'Left', [-0.8]))
    # logger2.warning('Jail zesty vixen who grabbed pay from quack.')
    # logger2.error('The five boxing wizards jump quickly.')
    time.sleep(0.5)
    

