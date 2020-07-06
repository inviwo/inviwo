# Inviwo Python script 
import inviwopy
import ivw.regression
import ivw.utils as inviwo_utils
import time


network = inviwopy.app.network;
lineWidth = network.LineRenderer.lineSettings.lineWidth;
# Line width will be set to two when the webpage has loaded
while (lineWidth.value != 2): 
    time.sleep(0.01);

inviwo_utils.update()
canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");

