#Inviwo Python script 
import inviwopy as ivw
import inviwopy.qt as iqt
from inviwopy.glm import *

network = ivw.app.network
factory = ivw.app.processorFactory;

network.clear()

#gridSize = 25
#dist = 75

p1 = factory.create('org.inviwo.NoiseProcessor',  ivec2(75 , -100))
p2 = factory.create('org.inviwo.ImageLowPass' , ivec2(75 , -25))
p3 = factory.create('org.inviwo.CanvasGL')
p4 = factory.create('org.inviwo.Background' , ivec2(0,-100)) # never added to network
p5 = factory.create('org.inviwo.VolumeSource' , ivec2(-100,-100))

p3.position = ivec2(75 , 50)


aProperty =  ivw.FloatProperty("test","Test Property" , 0.5 , 0 , 1 ,0.1 )

p2.addProperty( ivw.FloatProperty("test","Test Property" , 0.5 , 0 , 1 ,0.1 ))

network.addProcessor(p1)
network.addProcessor(p2)
network.addProcessor(p3)
network.addProcessor(p5)

network.addConnection( p1.outports[0] , p2.inports[0] )
network.addConnection( p2.outports[0] , p3.inports[0] )


for s in range(p2.kernelSize.minValue,p2.kernelSize.maxValue):
    p2.kernelSize.value = s
    iqt.update()

network.removeProcessor(p5)

p2.selected = True
