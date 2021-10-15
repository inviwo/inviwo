# Inviwo Python script 
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network;
isLoading = network.Webbrowser.isLoading;
# HACK: Wait for as little as possible while ensuring that the webpage has re-rendered. 
while isLoading.value:
	time.sleep(0.5);	
	inviwopy.qt.update();
canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");
