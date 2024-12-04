#Inviwo Python script 
import inviwopy
import time

start = time.time()

for i in range(0,100):
    inviwopy.app.network.GaussianVolumeRaycaster.camera.invalidate()

end = time.time()
print((end - start)/100 * 1000, "ms")