#
#  events.py
#  
#
#  Created by William Woodall on 2/19/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
from threading import Thread, Event, Lock
from Queue import Queue

import logging
log = logging.getLogger('Events')

event = Event()
lock = Lock()
queue = Queue(1024)

def push(e, msg):
	lock.acquire()
	if not queue.full():
		queue.put((e,msg))
		event.set()
	else:
		log.error("Event Queue is FULL!, dropping event %s %s" % (e, msg))
	lock.release()

def pop():
	temp = (None, None)
	lock.acquire()
	if not queue.empty():
		temp = queue.get()
		event.clear()
		if not queue.empty():
			event.set()
	else:
		log.error("Tried to pop when the queue was empty?  Check your logic, no event has happened, also check for race conditions.")
	lock.release()
	return temp
		
def triggerEvent(return_code, msg):
	push(return_code, msg)
	
def wait_for_event(timeout = 1):
	while not event.isSet():
		event.wait(timeout)

def clear_events():
	while not queue.empty():
		pop()
		
def get_last_event():
	return pop()
