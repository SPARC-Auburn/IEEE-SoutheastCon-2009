#
#  events.py
#  
#
#  Created by William Woodall on 2/19/09.
#  Copyright (c) 2009 Auburn University. All rights reserved.
#
from threading import Event
from Queue import Queue

global_event_lock = Event()

queue = Queue(1024)