# Inviwo Python script 
import inviwo 
import math 
import time

start = time.clock()

d = 70
steps = 360
for i in range(0, steps):
   r = (2 * 3.14 * i) / (steps-1)
   x = -d*math.sin(r)
   z = d*math.cos(r)
   inviwo.setPropertyValue("Point light source.lightPosition.position",(x,0,z))

end = time.clock()
fps = steps / (end - start)

fps = round(fps,2)

print ("fps: " + str(fps))