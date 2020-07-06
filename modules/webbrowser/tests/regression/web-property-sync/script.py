# Inviwo Python script 
import inviwopy
import ivw.regression
import ivw.utils as inviwo_utils
import time


network = inviwopy.app.network;
lineWidth = network.LineRenderer.lineSettings.lineWidth;
lineWidth.value = 2;
while (network.Webbrowser.isLoading.value): 
    time.sleep(0.01);

inviwo_utils.update()
canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");

