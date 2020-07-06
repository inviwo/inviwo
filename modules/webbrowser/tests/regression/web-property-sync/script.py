# Inviwo Python script 
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network;
lineWidth = network.LineRenderer.lineSettings.lineWidth;
lineWidth.value = 2;
while (network.Webbrowser.isLoading.value): 
    time.sleep(0.01);

canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");

