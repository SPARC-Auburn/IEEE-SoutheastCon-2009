/*
 *  lasermodule.c
 *  Sparc
 *
 *  Created by William Woodall on 9/15/08.
 *  Copyright 2008 Auburn University. All rights reserved.
 *
 */

#include <Python.h>

static char laserutil_doc[] = "This is a utility module for using the LRF, it provides FAST reading, translating, and manipulation of the laser data.";

static PyObject*
laserutil_scan(PyObject *self, PyObject *args) {
	int *fd = NULL;
	unsigned *start = NULL, *stop = NULL, *step = NULL;
	char command[10];
	
	if(!PyArg_UnpackTuple(args, "scan", 1, 4, &fd, &start, &stop, &step))
		return NULL;
	
	// Fill in defaults.
	if(fd == NULL)
		return NULL;
	if(start == NULL)
		*start = 0;
	if(stop == NULL)
		*stop = 768;
	if(step == NULL)
		*step = 0;
	
	
	// Check for errors.
	if(*start > 768 || *stop > 768 || *step > (*stop - *start) || *step > 99)
		return NULL;
	
	// Create the command
	command[0] = 'G';
	itoa(start, command[1], 10);
	itoa(stop, command[4], 10);
	itoa(step, command[7], 10);
	command[9] = '\r';
	
	// Write the command
	int n = write(*fd, &command, 10);
	
	// Read in the result
	char buffer[(((*stop-*start)%*step)+1)*2];
	int result[((*stop-*start)%*step)+1];
	char tl, th;
	
	// Burn the beginning response
	int i;
	for(i = 12; i > 0; i--)
		read(*fd, &tl, 1);
	
	// Begin reading
	for(i = 0; 1; i++) {
		read(*fd, &tl, 1);
		if(tl == '\n') {
			read(*fd, &tl, 1);
			if(tl == '\n') {
				break;
			}
			else {
				buffer[i] = tl;
			}
		}
		else {
			buffer[i] = tl;
		}
	}
	
	// Translate buffer
	for(i = 0; 1; i++) {
		if(buffer[i] == NULL)
			break;
		
	}
}

static char laserutil_scan_doc[] = 
"scan(fd, start, stop, step)\n\
\n\
Scans from start to stop with a value for every step number of readings.  Given the Fileno of the opened serial port.";

static PyMethodDef laserutil_methods[] = {
	{"scan", laserutil_scan, METH_VARARGS, laserutil_scan_doc},
	{NULL, NULL}
};

PyMODINIT_FUNC
initlaserutil(void) {
	Py_InitModule3("laserutil", laserutil_methods, laserutil_doc);
}
