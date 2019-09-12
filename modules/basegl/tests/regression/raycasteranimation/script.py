# Inviwo Python script 
import inviwopy
from inviwopy import qt as inviwoqt
from inviwopy.glm import size2_t,vec3

import math 
import time

import ivw.regression
import ivw.camera

steps = 50

m = ivw.regression.Measurements()

network = inviwopy.app.network;
canvas = network.CanvasGL;

orgsize = canvas.size;

with ivw.camera.Camera(network.EntryExitPoints.camera, lookfrom = vec3(0,4,0), lookto = vec3(0,0,0), lookup = vec3(0,0,1)) as c:
    for size in [256,512,768,1024]:
        canvas.size=size2_t(size, size)
        ivw.regression.saveCanvas(canvas, "CanvasGL-"+str(size) + "x" +str(size))

        start = time.perf_counter()
        for step in c.rotate(math.pi/steps, steps, vec3(0,1,0)):
            inviwoqt.update()
        end = time.perf_counter()
        frametime = (end - start) / steps
        fps = 1.0 / frametime
    
        m.addFrequency('FPS-'+str(size)+'x'+str(size), fps)

m.save()
canvas.size = orgsize;
