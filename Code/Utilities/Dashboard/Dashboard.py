from Tkinter import *
from PIL import Image
import Image, ImageTk
import copy

from Geometry import *
from RobotStatus import *
import SocketClass



HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port

def test(master, robot):
	msg = c.get_message()
	if not msg == '':
		text.insert(END, msg)
		text.insert(END, "\n")
	try:
		print int(msg)
		if int(msg) < 3:
			
			robot.Sorter.Position = int(msg)
			print 'Position = ', robot.Sorter.Position
			LRF_Component(master, robot)
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

def Arm_Component(master, robot):
	x, y = GeomObject(), GeomObject()
	x.orig, y.orig = 90, 50
	x.chg, y.chg = 10, -10
	x.end, y.end = 0.8*x.chg, 0.8*y.chg
	x.ofsA, y.ofsA = 8, 2
	x.ofsB, y.ofsB = 23, 38
	x.w, y.h = 30, 40
	
	canvas = Canvas(master, width=200, height=200)
	
	# Bottom of moving gripper piece: Closed
	x.p, y.p = x.orig+7, y.orig-12
	canvas.create_arc(x.p, y.p, x.p+x.w+3,y.p+y.h+4,start=90,extent=135,style="chord",fill="grey60",width=1)
	
	x.p, y.p = x.orig+7, y.orig-12
	x.ofsA, y.ofsA = 6, 5
	x.ofsB, y.ofsB = 5, 37
	canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
	x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="grey60")
	
	x.p, y.p = x.p+x.ofsB, y.p+y.ofsB
	canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=1)

	# 4 cylinders
	x.p, y.p = x.orig, y.orig
	canvas.create_oval(x.p+x.end, y.p+y.end, x.p+x.w+x.end, y.p+y.h+y.end, fill="#8080e0", width=1)
	canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA+x.end,  y.p+y.ofsA+y.end,
	x.p+x.ofsB+x.end,  y.p+y.ofsB+y.end,
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="#8080e0")
	canvas.create_oval(x.p, y.p, x.p+x.w, y.p+y.h, fill="#8080e0", width=1)
	
	x.p, y.p = x.p-x.chg, y.p-y.chg
	# canvas.create_oval(x.p+x.end, y.p+y.end, x.p+x.w+x.end, y.p+y.h+y.end, fill="#8080c8", width=1)
	# canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	# x.p+x.ofsA+x.end,  y.p+y.ofsA+y.end,
	# x.p+x.ofsB+x.end,  y.p+y.ofsB+y.end,
	# x.p+x.ofsB,  y.p+y.ofsB,
	# fill="#8080c8")
	# canvas.create_oval(x.p, y.p, x.p+x.w, y.p+y.h, fill="#8080c8", width=1)
	
	x.p, y.p = x.p-x.chg, y.p-y.chg
	# canvas.create_oval(x.p+x.end, y.p+y.end, x.p+x.w+x.end, y.p+y.h+y.end, fill="#8080a2", width=1)
	# canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	# x.p+x.ofsA+x.end,  y.p+y.ofsA+y.end,
	# x.p+x.ofsB+x.end,  y.p+y.ofsB+y.end,
	# x.p+x.ofsB,  y.p+y.ofsB,
	# fill="#8080a2")
	# canvas.create_oval(x.p, y.p, x.p+x.w, y.p+y.h, fill="#8080a2", width=1)
	
	x.p, y.p = x.p-x.chg, y.p-y.chg
	canvas.create_oval(x.p+x.end, y.p+y.end, x.p+x.w+x.end, y.p+y.h+y.end, fill="#808080", width=1)
	canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA+x.end,  y.p+y.ofsA+y.end,
	x.p+x.ofsB+x.end,  y.p+y.ofsB+y.end,
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="#808080")
	canvas.create_oval(x.p, y.p, x.p+x.w, y.p+y.h, fill="#808080", width=1)
	
	
	# # Moving gripper piece: Open
	# x.p, y.p = x.orig-8, y.orig-32
	# canvas.create_arc(x.p, y.p, x.p+x.w+3,y.p+y.h+4,start=0,extent=135,style="chord",fill="grey80",width=1)
	
	# x.ofsA, y.ofsA = x.ofsA, y.ofsA+1
	# x.ofsB, y.ofsB = x.ofsB+10, y.ofsB-16
	# canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	# x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
	# x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
	# x.p+x.ofsB,  y.p+y.ofsB,
	# fill="grey80")
	
	# x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
	# canvas.create_arc(x.p,y.p,x.p+x.w+3,y.p+y.h+4,start=0,extent=135,style="chord",fill="grey80",width=1)
	
	# x.p, y.p = x.orig, y.orig-29
	# canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=1)
	
	# Top of moving gripper piece
	x.p, y.p = x.orig+7, y.orig-12
	x.ofsA, y.ofsA = 6, 5
	x.ofsB, y.ofsB = 21, 0
	canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
	x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="grey80")
	
	x.p, y.p = x.orig+14, y.orig-8
	canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=1)
	
	x.p, y.p = x.orig+7, y.orig-12
	x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
	canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=82,extent=135,style="arc",width=1)
	# x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
	canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=90,extent=135,style="chord",fill="grey80",width=1)
	
	# Stationary gripper piece
	x.p, y.p = x.orig+9, y.orig-12
	canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=30,extent=60,style="chord",fill="grey80",width=1)
	
	
	# x.p, y.p = x.orig, y.orig
	# x.ofsA, y.ofsA = 28, -12
	# x.ofsB, y.ofsB = 40, -1
	x.ofsA, y.ofsA = 19, 0
	x.ofsB, y.ofsB = 31, 11
	canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA-(4*x.chg)+3,  y.p+y.ofsA-(4*y.chg)-3,
	x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="grey80")
	canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
	x.p+x.ofsB-(4*x.chg)+3,  y.p+y.ofsB-(4*y.chg)-3,
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="grey80")
	
	x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
	canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=30,extent=60,style="arc",fill="grey80",width=1)
	
	
	# # x.p, y.p = x.orig+7, y.orig-12
	# # canvas.create_arc(x.p, y.p, x.p+x.w+3,y.p+y.h+4,start=90,extent=135,style="pieslice",fill="grey80",width=1)
	
	
	# Gripper hinge
	x.p, y.p = x.orig+26, y.orig-12
	canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=2, fill="grey60")
	canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), dash=(4, 4), width=2)
	
	
	
	
	
	
	master.add(canvas)

def Drive_Component(master, robot):
	canvas = Canvas(master, width=200, height=200)
	
	canvas.img_arrowDown, canvas.img_speedGuageDown = PhotoImage(file = 'ArrowDown.gif'), PhotoImage(file = 'SpeedGuageDown.gif')
	canvas.img_arrowUp, canvas.img_speedGuageUp = PhotoImage(file = 'ArrowUp.gif'), PhotoImage(file = 'SpeedGuageUp.gif')
	canvas.img_speedScale = PhotoImage(file = 'SpeedScale.gif')
	canvas.create_image(40, 20, image = canvas.img_speedGuageUp, anchor = NW)
	canvas.create_image(35, -39, image = canvas.img_arrowUp, anchor = NW)
	canvas.create_image(10, 10, image = canvas.img_speedScale, anchor = NW)
	canvas.create_image(40+100, 20, image = canvas.img_speedGuageUp, anchor = NW)
	canvas.create_image(35+100, -39, image = canvas.img_arrowUp, anchor = NW)
	canvas.create_image(10+100, 10, image = canvas.img_speedScale, anchor = NW)
	# canvas.imageA = img_arrowUp
	# canvas.imageB = img_speedGuageUp
	# canvas.imageC = img_speedScale
	
	# canvas.create_polygon(-5,-5,-5,45,5,45,5,-5,fill="lightblue",outline="brown",width=1)
	# canvas.create_polygon(-10,45,-10,55,10,55,10,45,fill="lightblue",outline="brown",width=1)
	# canvas.create_polygon(-5+40,-5+40,-5+40,45+40,5+40,45+40,5+40,-5+40,fill="lightblue",outline="brown",width=1)
	# canvas.create_polygon(-10+40,45+40,-10+40,55+40,10+40,55+40,10+40,45+40,fill="lightblue",outline="brown",width=1)
	# canvas.create_text(40, 10, text='Motor', font=('verdana', 10, 'bold'))
	master.add(canvas)
	
def LRF_Component(master, robot):
	canvas = Canvas(master, width=200, height=150)
	img_sorter = PhotoImage(file = "SorterOptions.gif")
	img_selection = PhotoImage(file = "SorterSelectionFinal.gif")
	
	offset = 34 * robot.Sorter.Position - 34
	print 'Position = ', robot.Sorter.Position
	
	canvas.create_image(100, 100, image=img_sorter)
	canvas.create_image(100+offset, 100, image=img_selection)
	
	canvas.imageA = img_sorter
	canvas.imageB = img_selection
	master.add(canvas)


class Component(Frame):
	def __init__(self, master=None):
		self.canvas = Canvas(master, width=200, height=200)

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
left = Label(m1, text="left pane")

m1.add(left)
m2 = PanedWindow(m1, orient=VERTICAL,relief="groove",background="grey70")
m1.add(m2)

Arm_Component(m2, robot_status)
Drive_Component(m2, robot_status)
LRF_Component(m2, robot_status)




while 1:
	test(m2, robot_status)
	root.update()
	
	