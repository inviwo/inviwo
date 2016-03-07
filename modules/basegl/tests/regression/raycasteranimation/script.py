# Inviwo Python script 
import inviwo 
import inviwoqt
import math 
import time

import ivw.regression
import ivw.camera

steps = 50

m = ivw.regression.Measurements()
c = ivw.camera.Camera("EntryExitPoints.camera")

c.lookup = [0,0,1]
c.lookto = [0,0,0]
c.lookfrom = [0,4,0]
c.set()

for size in [256,512,768,1024]:
    inviwo.resizecanvas("CanvasGL", size, size)
    ivw.regression.saveCanvas("CanvasGL", "CanvasGL-"+str(size) + "x" +str(size))

    start = time.clock()
    c.rotate(math.pi/steps, steps, [0,1,0])
    end = time.clock()
    frametime = (end - start) / steps
    fps = 1.0 / frametime
    
    m.addFrequency('FPS-'+str(size)+'x'+str(size), fps)

m.save()
