# Inviwo Python script 
import inviwopy
from inviwopy import qt as inviwoqt
import math 
import time

import ivw.regression
import ivw.camera

steps = 50
network = inviwopy.app.network;

m = ivw.regression.Measurements()
for g in range(0,2):
    isGauss = g == 0;
    network.ImageLowPass.gaussian.value = isGauss
    name = "Gaussian-" if isGauss else "Box-"

    r = [1,3,5,10,15,30,32] if isGauss else [1,2,3,5,11,17,32,60]
    p = network.ImageLowPass.sigma if isGauss else  network.ImageLowPass.kernelSize

    for size in r:
        start = time.perf_counter()    
        p.value = size 
        inviwoqt.update()
        ivw.regression.saveCanvas(network.Lowpass, name+str(size) + "x" +str(size))
        end = time.perf_counter()
        m.addFrequency(name+'time-'+str(size) + "x" +str(size) , end - start );
m.save()
