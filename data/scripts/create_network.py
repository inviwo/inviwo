#Inviwo Python script 
import inviwopy as ivw
from inviwopy.glm import *

network = ivw.app.network
factory = ivw.app.processorFactory;

network.clear()

p1 = factory.create('org.inviwo.NoiseProcessor',  ivec2(75 , -100))
p2 = factory.create('org.inviwo.ImageLowPass' , ivec2(75 , -25))
p3 = factory.create('org.inviwo.CanvasGL')

p3.position = ivec2(75 , 50)

network.addProcessor(p1)
network.addProcessor(p2)
network.addProcessor(p3)

network.addConnection( p1.outports[0] , p2.inports[0] )
network.addConnection( p2.outports[0] , p3.inports[0] )


