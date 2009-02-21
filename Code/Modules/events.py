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

class Notifier(Thread):
	def __init__(self):
		Thread.__init__(self)
		self.stop = False
		
	def run(self):
		while not self.stop:
			if not queue.full() and not lock.isSet():
				global_event_lock.set()
			
	def shutdown(self):
		self.stop = True
		return
		
t = Notifier()
try:
	t.start()
finally:
	t.shutdown()
	t.join()