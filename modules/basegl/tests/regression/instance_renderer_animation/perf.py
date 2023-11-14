# Inviwo Python script
import inviwopy
from inviwopy import qt as inviwoqt
from inviwopy.glm import size2_t, size3_t, ivec2, vec3

import math
import time

import ivw.regression
import ivw.camera

steps = 25

measurement = ivw.regression.Measurements()

network = inviwopy.app.network

for size in [1024, 512, 256]:
    network.Canvas.size = size2_t(size)
   
    for np in [16, 8, 4]:
        network.PointGeneration.grid.nPoints.value = size3_t(np)
        
        for m in [64, 32, 16]:
            network.MeshCreator.res.value = ivec2(m)

            with ivw.camera.Camera(network.InstanceRenderer.camera,
                                   lookfrom=vec3(0, 8, 0),
                                   lookto=vec3(0, 0, 0),
                                   lookup=vec3(0, 0, 1)) as c:

                start = time.perf_counter()
                for step in c.rotate(math.pi / steps, steps, vec3(0, 1, 0)):
                    inviwoqt.update()
                end = time.perf_counter()
                frametime = (end - start) / steps
                fps = 1.0 / frametime

                print(f"FPS {size}x{size} instances {np**3} mesh resolution {m} {fps}")
        
                measurement.addFrequency(f"FPS size {size}x{size} instances {np**3} mesh resolution {m}", fps)

measurement.save()
