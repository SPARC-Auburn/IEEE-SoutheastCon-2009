from Tkinter import *
from Geometry import *
from RobotStatus import *
from PIL import Image
import Image, ImageTk
import copy
import math


# A list of functions for component initialization and update
# init functions are for unchanging graphics and 
# update functions are for changing some of the graphics according to input

'''
Laser Range Finder Sensor Component

'''
def init_LRF_Component(master, robot):
	lrf = robot.LRF
	lrf.half_w, lrf.half_h = 110, 110
	lrf.canvas = Canvas(master, width=lrf.half_w*2, height=lrf.half_h*2)
	master.add(lrf.canvas)
	
	update_LRF_Component(robot)

def update_LRF_Component(robot):
	lrf = robot.LRF
	lrf.canvas.delete("all")
	lrf.canvas.create_text(80, 10, text='Laser Range Finder', font=('verdana', 10, 'bold'))
	
	angle = 100.0
	n = 0
	for r in range(lrf.nRanges):
		lrf.coord[n] = (math.cos(angle * math.pi / 180) * 1.0 * lrf.range[r]) + lrf.half_w
		lrf.coord[n+1] = (math.sin(angle * math.pi / 180) * 1.0 * lrf.range[r]) + lrf.half_h
		
		angle += 1
		n += 2
		
	lrf.coord[-2] = lrf.half_w
	lrf.coord[-1] = lrf.half_h
		
	lrf.canvas.create_polygon(lrf.coord, fill="#8080e0", width=1, outline="#4040e0")
	center = Rhombus(lrf.half_w, lrf.half_h, 20, 20)
	lrf.canvas.create_polygon(center.coord, fill="grey40", width=2, outline="black")

'''
Hall Effect Sensor Component

'''
def init_HES_Component(master, robot):
	hes = robot.HES
	hes.canvas = Canvas(master, width=200, height=200)
	hes.img_scale = PhotoImage(file = "ScaleZ_P1.gif")
	master.add(hes.canvas)
	
	update_HES_Component(robot)
	
def update_HES_Component(robot):
	hes = robot.HES
	hes.canvas.delete("all")
	hes.canvas.create_text(80, 10, text='Hall Effect Sensors', font=('verdana', 10, 'bold'))
	
	robot_chassis = Rhombus(100, 100, 150, 75, -100)
	hes.canvas.create_polygon(robot_chassis.coord, fill="gray50")
	
	pos = [[180, 110],[40, 165],[130, 165]]	
	# A loop to create 3 Hall Effect guages and their 0.0 to 1.0 scales
	for s in range(3):
		hes.canvas.create_image(pos[s][0]-20, pos[s][1]-40, image=hes.img_scale)
		bar_y = Rhombus(pos[s][0], pos[s][1], 10, -80*hes.Sensor[s])
		bar_x = Rhombus(pos[s][0]+10, pos[s][1], 4, -80*hes.Sensor[s], 0, -3)
		bar_z = Rhombus(pos[s][0], pos[s][1]-80*hes.Sensor[s], 10, -3, 4)
		hes.canvas.create_polygon(bar_y.coord, fill="lightblue", width=1, outline="black")
		hes.canvas.create_polygon(bar_x.coord, fill="lightblue", width=1, outline="black")
		hes.canvas.create_polygon(bar_z.coord, fill="lightblue", width=1, outline="black")

'''
# Sorter Component

'''
def init_Sorter_Component(master, robot):
	sort = robot.Sorter
	sort.canvas = Canvas(master, width=200, height=150)
	sort.img_sorter = PhotoImage(file = "SorterOptions.gif")
	sort.img_selection = PhotoImage(file = "SorterSelectionFinal.gif")
	master.add(sort.canvas)
	
	update_Sorter_Component(robot)
	
def update_Sorter_Component(robot):
	sort = robot.Sorter
	sort.canvas.delete("all")
	sort.canvas.create_text(80, 10, text='Sorter Component', font=('verdana', 10, 'bold'))
	
	offset = 34 * sort.Position - 34
	
	sort.canvas.create_image(100, 100, image=sort.img_sorter)
	sort.canvas.create_image(100+offset, 100, image=sort.img_selection)

'''
Gripper Component
'''
def init_Gripper_Component(master, robot):
	grip = robot.Gripper
	grip.x, grip.y = GeomObject(), GeomObject()
	grip.x.orig, grip.y.orig = 90, 50
	grip.x.chg, grip.y.chg = 10, -10
	grip.x.end, grip.y.end = 0.8*grip.x.chg, 0.8*grip.y.chg
	grip.x.ofsA, grip.y.ofsA = 8, 2
	grip.x.ofsB, grip.y.ofsB = 23, 38
	grip.x.w, grip.y.h = 30, 40
	
	grip.canvas = Canvas(master, width=200, height=180)
	master.add(grip.canvas)
	
	update_Gripper_Component(robot)
	
def update_Gripper_Component(robot):
	grip = robot.Gripper
	grip.canvas.delete("all")
	grip.canvas.create_text(80, 10, text='Gripper Component', font=('verdana', 10, 'bold'))
	
	x, y = grip.x, grip.y
	
	# Bottom of moving gripper piece: Closed
	if grip.Status == Closed:
		x.p, y.p = x.orig+7, y.orig-12
		grip.canvas.create_arc(x.p, y.p, x.p+x.w+3,y.p+y.h+4,start=90,extent=135,style="chord",fill="grey60",width=1)
	
		x.p, y.p = x.orig+7, y.orig-12
		x.ofsA, y.ofsA = 6, 5
		x.ofsB, y.ofsB = 5, 37
		grip.canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
		x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
		x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
		x.p+x.ofsB,  y.p+y.ofsB,
		fill="grey60")
		
		x.p, y.p = x.p+x.ofsB, y.p+y.ofsB
		grip.canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=1)
	
	# 4 cylinders
	x.p, y.p = x.orig, y.orig
	color = ["#8080e0", "#8080c8", "#8080a2", "#808080"]
	
	n = 0
	for sensor in grip.Sensor:
		if sensor == On:
			grip.canvas.create_oval(x.p+x.end, y.p+y.end, x.p+x.w+x.end, y.p+y.h+y.end, fill=color[n], width=1)
			grip.canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
			x.p+x.ofsA+x.end,  y.p+y.ofsA+y.end,
			x.p+x.ofsB+x.end,  y.p+y.ofsB+y.end,
			x.p+x.ofsB,  y.p+y.ofsB,
			fill=color[n])
			grip.canvas.create_oval(x.p, y.p, x.p+x.w, y.p+y.h, fill=color[n], width=1)
			
		x.p, y.p = x.p-x.chg, y.p-y.chg
		n += 1	
	
	# Moving gripper piece: Open
	if grip.Status == Open:
		x.p, y.p = x.orig-8, y.orig-32
		grip.canvas.create_arc(x.p, y.p, x.p+x.w+3,y.p+y.h+4,start=0,extent=135,style="chord",fill="grey80",width=1)
		
		x.ofsA, y.ofsA = x.ofsA, y.ofsA+1
		x.ofsB, y.ofsB = x.ofsB+10, y.ofsB-16
		grip.canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
		x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
		x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
		x.p+x.ofsB,  y.p+y.ofsB,
		fill="grey80")
		
		x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
		grip.canvas.create_arc(x.p,y.p,x.p+x.w+3,y.p+y.h+4,start=0,extent=135,style="chord",fill="grey80",width=1)
		
		x.p, y.p = x.orig, y.orig-29
		grip.canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=1)
	
	# Top of moving gripper piece
	if grip.Status == Closed:
		x.p, y.p = x.orig+7, y.orig-12
		x.ofsA, y.ofsA = 6, 5
		x.ofsB, y.ofsB = 21, 0
		grip.canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
		x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
		x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
		x.p+x.ofsB,  y.p+y.ofsB,
		fill="grey80")
	
		x.p, y.p = x.orig+14, y.orig-8
		grip.canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=1)
	
		x.p, y.p = x.orig+7, y.orig-12
		x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
		grip.canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=82,extent=135,style="arc",width=1)
		# x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
		grip.canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=90,extent=135,style="chord",fill="grey80",width=1)
	
	# Stationary gripper piece
	x.p, y.p = x.orig+9, y.orig-12
	grip.canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=30,extent=60,style="chord",fill="grey80",width=1)
	
	# x.p, y.p = x.orig, y.orig
	# x.ofsA, y.ofsA = 28, -12
	# x.ofsB, y.ofsB = 40, -1
	x.ofsA, y.ofsA = 19, 0
	x.ofsB, y.ofsB = 31, 11
	grip.canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA-(4*x.chg)+3,  y.p+y.ofsA-(4*y.chg)-3,
	x.p+x.ofsB-(4*x.chg),  y.p+y.ofsB-(4*y.chg),
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="grey80")
	grip.canvas.create_polygon(x.p+x.ofsA, y.p+y.ofsA,
	x.p+x.ofsA-(4*x.chg),  y.p+y.ofsA-(4*y.chg),
	x.p+x.ofsB-(4*x.chg)+3,  y.p+y.ofsB-(4*y.chg)-3,
	x.p+x.ofsB,  y.p+y.ofsB,
	fill="grey80")
	
	x.p, y.p = x.p-(4*x.chg), y.p-(4*y.chg)
	grip.canvas.create_arc(x.p, y.p, x.p+x.w+3, y.p+y.h+4,start=30,extent=60,style="arc",fill="grey80",width=1)
	
	
	# # x.p, y.p = x.orig+7, y.orig-12
	# # canvas.create_arc(x.p, y.p, x.p+x.w+3,y.p+y.h+4,start=90,extent=135,style="pieslice",fill="grey80",width=1)
	
	
	# Gripper hinge
	x.p, y.p = x.orig+26, y.orig-12
	grip.canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), width=2, fill="grey60")
	grip.canvas.create_line(x.p, y.p, x.p-(4*x.chg), y.p-(4*y.chg), dash=(4, 4), width=2)

'''
Drive Component
'''
# def Drive_Component(master, robot):
	# canvas = Canvas(master, width=200, height=200)
	
	# canvas.img_arrowDown, canvas.img_speedGuageDown = PhotoImage(file = 'ArrowDown.gif'), PhotoImage(file = 'SpeedGuageDown.gif')
	# canvas.img_arrowUp, canvas.img_speedGuageUp = PhotoImage(file = 'ArrowUp.gif'), PhotoImage(file = 'SpeedGuageUp.gif')
	# canvas.img_speedScale = PhotoImage(file = 'ScaleN1_P1.gif')
	# canvas.create_image(40, 20, image = canvas.img_speedGuageUp, anchor = NW)
	# canvas.create_image(35, -39, image = canvas.img_arrowUp, anchor = NW)
	# canvas.create_image(10, 10, image = canvas.img_speedScale, anchor = NW)
	# canvas.create_image(40+100, 20, image = canvas.img_speedGuageUp, anchor = NW)
	# canvas.create_image(35+100, -39, image = canvas.img_arrowUp, anchor = NW)
	# canvas.create_image(10+100, 10, image = canvas.img_speedScale, anchor = NW)
	
	
	# # canvas.create_text(40, 10, text='Motor', font=('verdana', 10, 'bold'))
	# master.add(canvas)
	
def init_Drive_Component(master, robot):
	drive = robot.Motor
	drive.canvas = Canvas(master, width=200, height=200)
	drive.img_arrowDown, drive.img_speedGuageDown = PhotoImage(file = 'ArrowDown.gif'), PhotoImage(file = 'SpeedGuageDown.gif')
	drive.img_arrowUp, drive.img_speedGuageUp = PhotoImage(file = 'ArrowUp.gif'), PhotoImage(file = 'SpeedGuageUp.gif')
	drive.img_speedScale = PhotoImage(file = 'ScaleN1_P1.gif')
	
	master.add(drive.canvas)
	update_Drive_Component(robot)
	
def update_Drive_Component(robot):
	drive = robot.Motor
	drive.canvas.create_text(80, 10, text='Drive Component', font=('verdana', 10, 'bold'))
	
	img_guage, img_arrow = drive.img_speedGuageUp, drive.img_arrowUp
	offsetArrow, offsetGuage = -80*drive.Left, 0
	if drive.Left < 0:
		img_guage, img_arrow = drive.img_speedGuageDown, drive.img_arrowDown
		offsetArrow, offsetGuage = offsetArrow+100-1, 80+1
	drive.canvas.create_image(40, 20+offsetGuage-1, image = img_guage, anchor = NW)
	drive.canvas.create_image(35, offsetArrow+1, image = img_arrow, anchor = NW)
	drive.canvas.create_image(10, 10, image = drive.img_speedScale, anchor = NW)
	
	img_guage, img_arrow = drive.img_speedGuageUp, drive.img_arrowUp
	offsetArrow, offsetGuage = -80*drive.Right, 0
	if drive.Right < 0:
		img_guage, img_arrow = drive.img_speedGuageDown, drive.img_arrowDown
		offsetArrow, offsetGuage = offsetArrow+100-1, 80+1
	drive.canvas.create_image(40+100, 20+offsetGuage-1, image = img_guage, anchor = NW)
	drive.canvas.create_image(35+100, offsetArrow+1, image = img_arrow, anchor = NW)
	drive.canvas.create_image(10+100, 10, image = drive.img_speedScale, anchor = NW)

'''
Arm Component
'''
def init_Arm_Component(master, robot):
	arm = robot.Arm
	arm.canvas = Canvas(master, width=200, height=140)
	master.add(arm.canvas)
	
	update_Arm_Component(robot)
	
def update_Arm_Component(robot):
	arm = robot.Arm
	arm.canvas.delete("all")
	arm.canvas.create_text(70, 10, text='Arm Component', font=('verdana', 10, 'bold'))
	
	shaft = Quad([-5,-5,-5,55,5,55,5,-5])#(-5, -5, 45, 5)
	gripper = Quad([-15,55,-15,75,15,75,15,55])#(-10, 45, 10, 55)
	
	shaft.setAngle(arm.Angle)	
	gripper.setAngle(arm.Angle)	
	shaft.translate(100, 100)
	gripper.translate(100, 100)
	
	arm.canvas.create_polygon(shaft.coord_trans, fill="lightblue", width=1, outline="grey20")
	arm.canvas.create_polygon(gripper.coord_trans, fill="lightblue", width=1, outline="grey20")