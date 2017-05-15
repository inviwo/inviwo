# Inviwo Python script 
import inviwopy
from inviwopy.glm import vec3
import ivw.utils as inviwo_utils
import math 
import time

#works with the volumelighting_subclavia workspace

start = time.clock()

light = inviwopy.app.network.getProcessorByIdentifier("Point light source").lightPosition.position

d = 70
steps = 360
rotations = 4
for i in range(0, steps):
    r = rotations*(2 * 3.14 * i) / (steps-1)
    x = -d*math.sin(r)
    z = d*math.cos(r)
    light.value = vec3(x,0,z)
    inviwo_utils.update() # Needed for canvas to update

end = time.clock()
fps = steps / (end - start)

fps = round(fps,2)

print ("fps: " + str(fps))