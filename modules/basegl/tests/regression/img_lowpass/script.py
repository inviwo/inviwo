# Inviwo Python script 
import inviwo 
import inviwoqt
import math 
import time

import ivw.regression
import ivw.camera

steps = 50

m = ivw.regression.Measurements()


for kernelSize in range(1,25,2):
    start = time.clock()    
    inviwo.setPropertyValue("Image Low Pass.kernelSize",kernelSize)
    inviwoqt.update()
    ivw.regression.saveCanvas("Lowpass", "Lowpass-"+str(kernelSize) + "x" +str(kernelSize))
    end = time.clock()
    m.addFrequency('time-'+str(kernelSize) + "x" +str(kernelSize) , end - start );



m.save()
