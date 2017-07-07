# Inviwo Python script 
import inviwopy
from inviwopy.glm import vec3
import ivw.utils as inviwo_utils
import math 
import time

#Use for example the boron example network

app = inviwopy.app
network = app.network

cam = network.EntryExitPoints.camera

start = time.clock()

scale = 1;
d = 15
steps = 120
cam.lookTo = vec3(0,0,0)
cam.lookUp = vec3(0,1,0)
for i in range(0, steps):
    r = (2 * math.pi * i) / (steps-1)
    x = d*math.sin(r)
    z = -d*math.cos(r)
    cam.lookFrom = vec3(x*scale,3*scale,z*scale)
    inviwo_utils.update() # Needed for canvas to update


for i in range(0, steps):
   r = (2 * math.pi * i) / (steps-1)
   x = 1.0*math.sin(r)
   z = 1.0*math.cos(r)
   cam.lookUp = vec3(x*scale,z*scale,0)
   inviwo_utils.update() # Needed for canvas to update

end = time.clock()
fps = 2*steps / (end - start)

fps = round(fps,3)

print("Frames per second: " + str(fps))
print("Time per frame: " + str(round(1000/fps,1)) + " ms")