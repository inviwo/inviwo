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


for kernelSize in range(1,25,2):
    start = time.clock()    
    network.ImageLowPass.kernelSize.value = kernelSize 
    inviwoqt.update()
    ivw.regression.saveCanvas(network.Lowpass, "Lowpass-"+str(kernelSize) + "x" +str(kernelSize))
    end = time.clock()
    m.addFrequency('time-'+str(kernelSize) + "x" +str(kernelSize) , end - start );
m.save()
