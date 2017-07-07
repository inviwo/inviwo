#Inviwo Python script 

import inviwopy
from inviwopy.glm import *


app = inviwopy.app
network = app.network

network.clear()

volProcessor = app.processorFactory.create('org.inviwo.VectorFieldGenerator3D', ivec2(0,0))
infoProcessor = app.processorFactory.create('org.inviwo.VolumeInformation', ivec2(0,100))

network.addProcessor(volProcessor)
network.addProcessor(infoProcessor)

network.addConnection(     volProcessor.outports[0]  , infoProcessor.inports[0]  )


volProcessor.xRange.range = vec2(0,1)
volProcessor.yRange.range = vec2(0,1)
volProcessor.zRange.range = vec2(0,1)
volProcessor.x.value = 'x*15'
volProcessor.y.value = 'y*15'
volProcessor.z.value = 'z*15'

volPort = volProcessor.outports[0]
volume = volPort.data


print(volume.dimensions)
data = volume.data

print(data[3,4,5])