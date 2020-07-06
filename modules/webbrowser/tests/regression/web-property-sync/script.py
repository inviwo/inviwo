# Inviwo Python script 
import inviwopy
import ivw.regression

network = inviwopy.app.network;
lineWidth = network.LineRenderer.lineSettings.lineWidth;
lineWidth.value = 2;

canvas = network.Canvas;

ivw.regression.saveCanvas(canvas, "Canvas");

