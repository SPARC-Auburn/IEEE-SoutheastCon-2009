import random

positions = random.sample(range(0,82),10)

for x in positions:
	print 'Row: '.rjust(10) + repr(x/9).rjust(10) + 'Column: '.rjust(10) + repr(x%9).rjust(10)
