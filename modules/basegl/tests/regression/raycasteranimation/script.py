# Inviwo Python script 
import inviwo 
import inviwoqt
import math 
import time

import ivw.regression
import ivw.camera

steps = 50

m = ivw.regression.Measurements()

with ivw.camera.Camera("EntryExitPoints.camera", lookfrom = [0,4,0], lookto = [0,0,0], lookup = [0,0,1]) as c:
    for size in [256,512,768,1024]:
        inviwo.resizecanvas("CanvasGL", size, size)
        ivw.regression.saveCanvas("CanvasGL", "CanvasGL-"+str(size) + "x" +str(size))

        start = time.clock()
        for step in c.rotate(math.pi/steps, steps, [0,1,0]):
            inviwoqt.update()
        end = time.clock()
        frametime = (end - start) / steps
        fps = 1.0 / frametime
    
        m.addFrequency('FPS-'+str(size)+'x'+str(size), fps)

m.save()
