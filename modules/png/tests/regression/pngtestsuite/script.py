# Inviwo Python script
import inviwopy
import time
import os

import ivw.regression
import ivw.camera

network = inviwopy.app.network

m = ivw.regression.Measurements()

canvas = network.canvas
imgsrc = network.imgsrc

# These images cannot be compared properly in the image comparison.
# Some images with a color palette will be converted to RGB while the test image is RGBA
PILNotWorking = ["tbgn2c16.png",
                 "tbrn2c08.png",
                 "basi0g01.png",
                 "tbbn2c16.png",
                 "basn0g01.png",
                 "tbbn0g04.png",
                 "tbwn0g16.png",
                 "tbbn3p08.png",
                 "tbgn3p08.png",
                 "tbwn3p08.png",
                 "tbyn3p08.png",
                 "tm3n3p02.png",
                 "tp1n3p08.png",]

path = inviwopy.app.getModuleByIdentifier("png").path + "/tests/pngtestimages/"
for filename in os.listdir(path):
    if filename in PILNotWorking:
        continue
    if filename.endswith(".png"):
        start = time.perf_counter()
        imgsrc.imageFileName.value = path + filename

        size = imgsrc.imageDimension_.value

        canvas.inputSize.dimensions.minValue = size
        canvas.inputSize.dimensions.maxValue = size
        canvas.inputSize.dimensions.value = size

        canvas.inputSize.customInputDimensions.minValue = size
        canvas.inputSize.customInputDimensions.maxValue = size
        canvas.inputSize.customInputDimensions.value = size
        ivw.regression.saveCanvas(canvas, filename[0:-4])
        end = time.perf_counter()
        m.addFrequency(filename + '-time', end - start)

m.save()

# make sure the last image is always the same (independent on os.listdir order)
imgsrc.imageFileName.value = path + "basi0g02.png"
size = imgsrc.imageDimension_.value

canvas.inputSize.dimensions.minValue = size
canvas.inputSize.dimensions.maxValue = size
canvas.inputSize.dimensions.value = size

canvas.inputSize.customInputDimensions.minValue = size
canvas.inputSize.customInputDimensions.maxValue = size
canvas.inputSize.customInputDimensions.value = size
