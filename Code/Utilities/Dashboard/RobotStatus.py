
Open, Closed = range(2)
On, Off = range(2)
Connected, Disconnected = range(2)

class Component():
	def __init__(self):
		self.display = True

class RobotStatus():
	def __init__(self):
	
		# A variable to keep track of whether a connection is made with the robot
		self.Connection = Disconnected
		
		# A variable to keep track of what routines are being executed
		self.RoutineQueue = []
		
		# The robot needs to maintain the status of the following components:
		# L/R Motors, Arm, Gripper, Sorter, LRF and the Hall effect sensors.
		self.Arm = Component()
		self.Gripper = Component()
		self.Sorter = Component()
		self.Motor = Component()
		self.LRF = Component()
		self.HES = Component()
		
		self.resetArm()
		self.resetGripper()
		self.resetSorter()
		self.resetMotor()
		self.resetLRF()
		self.resetHES()
		
		
	def resetArm(self):
		self.Arm.Angle = 0.0
		
	def resetGripper(self):
		self.Gripper.Status = Closed
		self.Gripper.Sensor = [On, Off, Off, On]
		
	def resetSorter(self):
		self.Sorter.Position = 0
		
	def resetMotor(self):
		self.Motor.Left = -0.25
		self.Motor.Right = 0.5
		self.Motor.Facing = 90.0
		
	def resetLRF(self):
		# Set to max distance
		self.LRF.veiwingfield = 220
		self.LRF.nRanges = 220
		self.LRF.AngleVariation = 1.0 * self.LRF.veiwingfield / self.LRF.nRanges
		
		self.LRF.range = []
		self.LRF.coord = []
		for x in range(self.LRF.nRanges+1):
			self.LRF.range.append(100)
			self.LRF.coord.append(0)
			self.LRF.coord.append(0)
			
		self.LRF.range[self.LRF.nRanges] = 0
		
	def resetHES(self):
		self.HES.Sensor = [0.6, 0.6, 0.6]
		