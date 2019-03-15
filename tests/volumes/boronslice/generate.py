#Inviwo Python script 
import inviwopy


app = inviwopy.app
network = app.network

base = "C:/Users/petst55.AD/Documents/Inviwo/inviwo/tests/volumes/boronslice/boron{:03}.png"

for i in range(0,150):
    network.VolumeSlice.sliceNumber.value = i
    inviwopy.qt.update()
    network.Canvas2.snapshot(base.format(i))