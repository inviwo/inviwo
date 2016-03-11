# Inviwo Python script 
import inviwo 
import inviwo_utils
import math 
import time

start = time.clock()

scale = 1;
d = 15
steps = 120
for i in range(0, steps):
   r = (2 * 3.14 * i) / (steps-1)
   x = d*math.sin(r)
   z = -d*math.cos(r)
   inviwo.setPropertyValue("EntryExitPoints.camera",((x*scale,3*scale,z*scale),(0,0,0),(0,1,0)))
   inviwo_utils.update() # Needed for canvas to update


for i in range(0, steps):
   r = (2 * 3.14 * i) / (steps-1)
   x = 1.0*math.sin(r)
   z = 1.0*math.cos(r)
   inviwo.setCameraUp("EntryExitPoints.camera",(x*scale,z*scale,0))
   inviwo_utils.update() # Needed for canvas to update

end = time.clock()
fps = 2*steps / (end - start)

fps = round(fps,3)

print("Frames per second: " + str(fps))
print("Time per frame: " + str(round(1000/fps,1)) + " ms")