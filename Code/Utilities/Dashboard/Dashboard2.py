from Tkinter import *
from PIL import Image
import Image, ImageTk, tkMessageBox
import copy

from Constants import *
from Geometry import *
from RobotStatus import *
from InterfaceComponents import *
import Networking

#HOST = '127.0.0.1'			# Loopback
#PORT = 50007				# Random port
updateScrollPosition = False

def terminate():
    # if tkMessageBox.askokcancel("Quit", "Do you really wish to quit?"):
    root.destroy()
		

def connect():
	robot_status.Connection = Connected
	try:
		conn = Networking.Client('111.1.1.1', PORT)
		#print 'Connection established to host ' + HOST + '.'
		text.insert(END, 'Connection established to host ' + HOST + '.')
		text.insert(END, "\n")
	except:
		conn = None
		robot_status.Connection = Disconnected
		tkMessageBox.showinfo(title=None, message="Unable to connect to host at " + HOST + ".")
		
	return conn
	
def disconnect():
	robot_status.Connection = Disconnected
	
	return None
	
def test(robot):
	event = conn.get_message()
	if event != '':
		print event
		msg = event.strip('<>').split(',')
		msg[-1] = msg[-1].strip('" ')
		eventDataStr = msg[-1].split(':')
		
		# Print message to the event viewer
		if not msg == []:
			text.insert(END, msg[-1])
			text.insert(END, "\n")
			if updateScrollPosition:
				text.yview(MOVETO, 1.0)
        
		# Update the component graphic
		component, attribute = eventDataStr[0], eventDataStr[1]
		del eventDataStr[0:2]
		
		eventDataReal = []
		for s in eventDataStr:
			eventDataReal.append(float(s))
		
		updateAttr(robot_status, component, attribute, eventDataReal)
		

def Event_Viewer(master):
	# Device component Panel Dashboard console status condition meter monitor: history event
	Events_Window = Toplevel(root)
	#Events_Window.transient(root)
	Events_Window.title("Event History")
	#Label(Events_Window, text="This is a regular toplevel window.").grid()
	
	text = Text(Events_Window, height=26, width=50)
	scroll = Scrollbar(Events_Window, command=text.yview)
	text.pack(side=LEFT, fill=Y)
	scroll.pack(side=RIGHT, fill=Y)
	scroll.config(command=text.yview)
	text.config(yscrollcommand=scroll.set)
	return text


def define_menu(master):
	basemenu = Menu(master)
	
	master.add_cascade(label="File", menu=basemenu)
	basemenu.add_command(label="Connect to host",command=connect)
	basemenu.add_command(label="Disconnect from host",command=disconnect)
	basemenu.add_separator()
	basemenu.add_command(label="Exit", command=terminate)
	
	return basemenu


#-- Create user interface application
root = Tk()

#-- Create secondary window for recording events with root as parent
text = Event_Viewer(root)

#-- Create the menu frame, and add menus to the menu frame
root.title("Dashboard")
menu_frame = Frame(root)
menu_frame.pack(fill=X, side=TOP)

menu = Menu(root)
root.config(menu=menu)
define_menu(menu)
# menu_frame.tk_menuBar(help_menu())

#-- Create the components frame
component_frame = Frame(root)
component_frame.pack(fill=X, side=TOP)


m1 = PanedWindow(component_frame,relief="groove",background="grey70")
m1.pack(fill=BOTH, expand=1)
# left = Label(m1, text="left pane")


column1 = PanedWindow(m1, orient=VERTICAL,relief="groove",background="grey70")
column2 = PanedWindow(m1, orient=VERTICAL,relief="groove",background="grey70")
m1.add(column1)
m1.add(column2)

robot_status = RobotStatus()

init_Component_LRF(column1, robot_status)
init_Component_HES(column1, robot_status)
init_Component_Sorter(column1, robot_status)

init_Component_Gripper(column2, robot_status)
init_Component_Drive(column2, robot_status)
init_Component_Arm(column2, robot_status)

conn = connect()

while 1:
	#if robot_status == Connected:
		#print "connected"
	test(robot_status)
	root.update()
	
	
