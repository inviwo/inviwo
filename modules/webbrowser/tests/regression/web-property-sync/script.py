# Inviwo Python script 
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network;
lineWidth = network.LineRenderer.lineSettings.lineWidth;
# Line width will be set to two when the webpage has loaded
while (lineWidth.value != 2): 
    inviwopy.qt.update();
inviwopy.qt.update();
canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");

