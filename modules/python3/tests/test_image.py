#Inviwo Python script 
import inviwopy
from inviwopy.glm import *

#import numpy as np
#from ivw_numpy import image_utils


#Load the boron network

app = inviwopy.app
network = app.network

canvas = network.CanvasGL
img = canvas.image
layer = img.colorLayers[0]
data = layer.data

#print(type(data))
#print(dir(data))


