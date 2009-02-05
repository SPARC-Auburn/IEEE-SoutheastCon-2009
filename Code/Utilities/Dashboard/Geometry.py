# from Tkinter import *
from math import *
import copy


class GeomObject():
	def __init___():
		self.p = 0
		self.orig = 0
		self.chg = 0
		self.end = 0
		self.ofsA = 0
		self.ofsB = 0
		self.w = 0
		self.h = 0


class Quad():
	coord, coord_trans = [], []
	def __init__(self, *args):#x_start=0, y_start=0, x_end=0, y_end=0):
		self.rotation = 0
		self.coord, self.coord_trans = [], []
		if len(*args) == 8:
			for x in args[0]:
				# print x
				self.coord.append(x)
				self.coord_trans.append(x)
		else:
			print 'Only ', len(*args),' arguments passed. 8 are needed.'
		
		
	def renew(self):
		self.rotation = 0
		self.coord_trans = copy.copy(self.coord)
		
	def setAngle(self, degree):
		self.rotation = degree

		index = 0
		for index in range(4):
			x = self.coord[index*2]
			y = self.coord[index*2+1]
			magnitude = (x**2 + y**2)**0.5
			angle = degrees( atan2(y,x) ) + degree
			
			self.coord_trans[index*2] = cos( radians(angle) )*magnitude
			self.coord_trans[index*2+1] = sin( radians(angle) )*magnitude
			
	def rotate(self, degree):
		self.setAngle(self.Rotation+degree)
			
	def translate(self, x_offset=0, y_offset=0):
		for index in range(4):
			self.coord_trans[index*2] += x_offset
			self.coord_trans[index*2+1] += y_offset
	
	def printCoords(self):
		print self.coord_trans
			

# A quad is an object with 4 points that can be rotated, scaled, and translated
# and also keeps track of its original state
class Square():
	def __init__(self, x_start=0, y_start=0, x_end=0, y_end=0):
		
		self.coord.extend([ x_start, y_start ])
		self.coord.extend([ x_end, y_start ])
		self.coord.extend([ x_end, y_end ])
		self.coord.extend([ x_start, y_end ])
		
		self.coord_trans = copy.copy(self.coord)

			
class Rhombus():
	def __init__(self, x, y, w, h, x_shift_in=0, y_shift_in=0):
		self.x_shift = x_shift_in
		self.y_shift = y_shift_in
		
		self.coord, self.coord_trans = [], []
		self.coord.extend([ x, y ] )
		self.coord.extend([ x + w, y + y_shift_in ])
		self.coord.extend([ x + w + x_shift_in, y + h + y_shift_in ])
		self.coord.extend([ x + x_shift_in, y + h ])
		
		self.coord_trans = copy.copy(self.coord)
	
