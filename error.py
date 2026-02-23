#Inviwo Python script 
import inviwopy


app = inviwopy.app
network = app.network
pic = network.Canvas

width,height = pic.size
for x in range(width):
	for y in range(height):
		r,g,b = pic.getpixel(x,y)