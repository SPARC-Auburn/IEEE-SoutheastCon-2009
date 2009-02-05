from Tkinter import *
from PIL import Image
import Image, ImageTk
import copy

from Geometry import *
from RobotStatus import *
from InterfaceComponents import *
import SocketClass

HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port

def test(robot):
	msg = c.get_message()
	if not msg == '':
		text.insert(END, msg)
		text.insert(END, "\n")
	try:
		print int(msg)
		if int(msg) < 4:
			
			robot.Sorter.Position = int(msg)-1
			print 'Position = ', robot.Sorter.Position
			update_Sorter_Component(robot)
	except:
		msg = ''
		

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


def help_menu():
    help_btn = Menubutton(menu_frame, text='Help', underline=0)
    help_btn.pack(side=LEFT, padx="2m")
    help_btn.menu = Menu(help_btn)
    help_btn.menu.add_command(label="How To", underline=0)#, command=HowTo)
    help_btn.menu.add_command(label="About", underline=0)#, command=About)
    help_btn['menu'] = help_btn.menu
    return help_btn



robot_status = RobotStatus()
c = SocketClass.Client(HOST, PORT)

#-- Create user interface application
root = Tk()

#-- Create secondary window for recording events with root as parent
text = Event_Viewer(root)

#-- Create the menu frame, and add menus to the menu frame
root.title("Dashboard")
menu_frame = Frame(root)
menu_frame.pack(fill=X, side=TOP)
menu_frame.tk_menuBar(help_menu())

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


init_LRF_Component(column1, robot_status)
init_HES_Component(column1, robot_status)
init_Sorter_Component(column1, robot_status)

init_Gripper_Component(column2, robot_status)
init_Drive_Component(column2, robot_status)
init_Arm_Component(column2, robot_status)




while 1:
	test(robot_status)
	root.update()
	
	