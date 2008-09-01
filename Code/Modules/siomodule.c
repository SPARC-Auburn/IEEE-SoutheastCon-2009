#include <Python.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

static char sio_doc[] = 
"This module allows you to open, read, write, and close serial devices in Linux.";

static PyObject*
sio_open(PyObject *self, PyObject *args)
{
	const char *device;
		
	if (!PyArg_ParseTuple(args, "s", &device)) {
		return NULL;
	}

	int fd; /* File descriptor for the port */


      fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
      if (fd == -1)
      {
       /*
		* Could not open the port.
		*/

		  perror("open_port: Unable to open device.");
	  }
	  else
		  fcntl(fd, F_SETFL, 0);

	return Py_BuildValue("i", fd);
}

static char sio_open_doc[] = 
"Opens a serial port given the device name.\nopen(string device_name);\n";

static PyObject*
sio_read(PyObject *self, PyObject *args)
{
	char buffer[1024];
	char *bufptr;
	int *fd;
	
	if(!PyArg_ParseTuple(args, "i", &fd))
		return NULL;
	
	read(*fd, bufptr, buffer + sizeof(buffer) - bufptr - 1);
	
	return Py_BuildValue("s", buffer);
}

static char sio_read_doc[] =
"Reads data from the serial port:\nread(int fd);\n";

static PyObject*
sio_write(PyObject *self, PyObject *args)
{
	int *fd, *size;
	const char *msg;
	
	if(!PyArg_ParseTuple(args, "isi", &fd, &msg, &size))
		return NULL;
	
	int n = write(*fd, msg, *size);
	if(n < 0)
		fputs("write() failed!", stderr);
	
	return NULL;
}

static char sio_write_doc[] = 
"Writes a char to the serial port given:\nwrite(int fd, string msg);\n";

static PyObject*
sio_close(PyObject *self, PyObject *args)
{
	int *fd;
	
	//if(!PyArg_ParseTuple(args, "i", &fd))
	//	return NULL;
	
	//close(*fd);
	
	return NULL;
}

static char sio_close_doc[] =
"Closes the serial port\nclose(int fd);\n";

static PyMethodDef sio_methods[] = {
	{"open", sio_open, METH_VARARGS, sio_open_doc},
	{"read", sio_read, METH_VARARGS, sio_read_doc},
	{"write", sio_write, METH_VARARGS, sio_write_doc},
	{"close", sio_close, METH_VARARGS, sio_close_doc},
	{NULL, NULL}
};

PyMODINIT_FUNC
initsio(void)
{
	Py_InitModule3("sio", sio_methods, sio_doc);
}
