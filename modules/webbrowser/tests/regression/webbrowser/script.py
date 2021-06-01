# Inviwo Python script 
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network;
# HACK: Wait for as little as possible while ensuring that the webpage has re-rendered. 
time.sleep(10);
inviwopy.qt.update();
canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");
