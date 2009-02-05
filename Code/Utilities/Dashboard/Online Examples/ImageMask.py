def automask(im):
    width, height = im.size
    mask = Image.new(im.size, '1') # black and white
    tran = im.getpixel((0,0))[0]   # transparent top-left
    for y in range(height):
        line = im.getpixel([(x, y) for x in range(width)])
        for x in range(width):
            if line[x] == tran:
                mask.point((x,y), 0)  # mask on the point
    return mask
	
	
from graphics import Image
 
ship = Image.open('E:\\Images\\ship.gif')
mask = automask(ship)
 
canvas.blit(ship, mask=mask)  # don't forget to create canvas first