from Tkinter import *
from appuifw import *
from PIL import Image
import Image, ImageTk
import SocketClass

HOST = '127.0.0.1'			# Loopback
PORT = 50007				# Random port

# class Quad(x_start=0, x_end=0, y_start=0, y_end=0):
	# def __init___():
		# self.rotation = 0
		# self.coord, self.coord_trans = [None, None, None, None]
		# self.coord[0], self.coord_trans[0] = [x_start,	y_start	]
		# self.coord[1], self.coord_trans[1] = [x_start,	y_end	]
		# self.coord[2], self.coord_trans[2] = [x_end, 	y_end	]
		# self.coord[3], self.coord_trans[3] = [x_end,	y_start	]
		
	# def renew():
		# self.rotation = 0
		# for index, c in self.coord[ c ]:
			# self.coord_trans[ index ] = self.coord[ index ]
		
	# def rotate(degree=0):
		# self.rotation = degree
		# for index, c in self.coord_trans:
			# c[ 0 ] = c[ 0 ]
			# c[ 1 ] = c[ 1 ]
			
	# def translate(x_offset=0, yoffset=0):
		# for index, c in self.coord_trans:
			# c[ 0 ] = c[ 0 ] + x_offset
			# c[ 1 ] = c[ 1 ] + y_offset
			

def test():
	msg = c.get_message()
	if not msg == '':
		text.insert(END, msg)
		text.insert(END, "\n")

def Event_Veiwer(master):

        Label(master, text="First:").grid(row=0)
        Label(master, text="Second:").grid(row=1)

        self.e1 = Entry(master)
        self.e2 = Entry(master)

        self.e1.grid(row=0, column=1)
        self.e2.grid(row=1, column=1)
        return self.e1 # initial focus
			

def Arm_Component(master):
	xy = [10, 90]
	xy_change = [10, -10]
	cylinder_w = 5
	cylinder_h = 20
	canvas = Canvas(master, width=200, height=200)
	# canvas.create_line(0, 0, 200, 100)
	# canvas.create_line(0, 100, 200, 0, fill="red", dash=(4, 4))
	# canvas.create_rectangle(50, 25, 150, 75, fill="blue")
	# circle(x,y,radius=5, outline=0, fill=0xffff00, width=1):
	canvas.ellipse((xy[0], xy[1], xy[0]+cylinder_w, xy[1]+radius), outline=0, fill=0xffff00, width=1)
	master.add(canvas)

def Drive_Component(master):
	canvas = Canvas(master, width=200, height=200)
	img_speedGuage = PhotoImage(file = 'SpeedGuage.gif')
	img_arrow = PhotoImage(file = 'arrow.gif')
	canvas.create_image(10, 20, image = img_speedGuage, anchor = NW)
	canvas.create_image(5, -80, image = img_arrow, anchor = NW)
	canvas.imageB = img_speedGuage
	canvas.imageA = img_arrow
	
	# canvas.create_polygon(-5,-5,-5,45,5,45,5,-5,fill="lightblue",outline="brown",width=1)
	# canvas.create_polygon(-10,45,-10,55,10,55,10,45,fill="lightblue",outline="brown",width=1)
	# canvas.create_polygon(-5+40,-5+40,-5+40,45+40,5+40,45+40,5+40,-5+40,fill="lightblue",outline="brown",width=1)
	# canvas.create_polygon(-10+40,45+40,-10+40,55+40,10+40,55+40,10+40,45+40,fill="lightblue",outline="brown",width=1)
	master.add(canvas)
	
def LRF_Component(master):
	canvas = Canvas(master, width=200, height=200)
	img_sorter = PhotoImage(file = 'SorterImageSmall.gif')
	img_selection = PhotoImage(file = 'SorterSelectionFinal.gif')
	# img_selection.load()
	# x = img_selection.getdata()
	# text.insert(END, img_selection[0][0])
	# img_sorter = Image.open("SorterImageSmall.gif")
	# img_selection = Image.open("SorterSelectionFinal.gif")
	# put gif image on canvas
	# pic's upper left corner (NW) on the canvas is at x=50 y=10
	# canvas.create_image(10, 10, image = img_sorter, anchor = NW)
	# canvas.create_image(10, 10, image = img_selection, anchor = NW)
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



root = Tk()

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

# for i in range(40): 
   # text.insert(END, "This is line %d\n" % i)
# text.delete(1.0, END)

#-- Create the menu frame, and add menus to the menu frame
root.title("Dashboard")
menu_frame = Frame(root)
menu_frame.pack(fill=X, side=TOP)
menu_frame.tk_menuBar(help_menu())
# e = Entry(root)
# e.pack()

#-- Create the status frame
status_frame = Frame(root)
status_frame.pack(fill=X, side=TOP)

m1 = PanedWindow(status_frame)
m1.pack(fill=BOTH, expand=1)
left = Label(m1, text="left pane")


# # im = Image.open("BTNShade.png")
# gif1 = PhotoImage(file = 'Guardian.gif')
# # put gif image on canvas
# # pic's upper left corner (NW) on the canvas is at x=50 y=10
# canvas.create_image(50, 10, image = gif1, anchor = NW)


m1.add(left)
m2 = PanedWindow(m1, orient=VERTICAL)
m1.add(m2)

Arm_Component(m2)
Drive_Component(m2)
LRF_Component(m2)


c = SocketClass.Client(HOST, PORT)

while 1:
	test()
	root.update()
	
	