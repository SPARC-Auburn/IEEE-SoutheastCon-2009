#
#  events.py
#  
#
#  Created by William Woodall on 2/19/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
from threading import Thread, Event
from Queue import Queue

lock = Event()

queue = Queue(1024)

def push(e, msg):
	if not queue.full():
		queue.put((e,msg))

def pop():
	if not queue.empty():
		temp = queue.get()
		lock.clear()
		return temp
		
def triggerEvent(return_code, msg):
	push(return_code, msg)
	
def wait_for_event(timeout = 1):
	while not lock.isSet():
		lock.wait(timeout)
		
def get_last_event():
	return pop()

class Notifier(Thread):
	def __init__(self):
		Thread.__init__(self)
		self.stop = False
		
	def run(self):
		while not self.stop:
			if not queue.empty() and not lock.isSet():
				lock.set()
			
	def shutdown(self):
		self.stop = True
		return
		
t = Notifier()
t.start()