# Inviwo Python script 
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network;
lineWidth = network.LineRenderer.lineSettings.lineWidth;
# Line width will be set to two when the webpage has loaded
while (lineWidth.value != 2): 
    inviwopy.qt.update();
# HACK: Wait for as little as possible while ensuring that the webpage has re-rendered. 
time.sleep(5.0);
inviwopy.qt.update();
canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");

