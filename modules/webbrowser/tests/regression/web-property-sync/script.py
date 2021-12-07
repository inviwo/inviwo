# Inviwo Python script
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network

# HACK: Wait for as little as possible while ensuring that the webpage has re-rendered.
isLoading = network.Webbrowser.isLoading
inviwopy.logInfo(f"is loading: {isLoading.value}")
while isLoading.value:
    inviwopy.logInfo(f"Waiting for webpage {isLoading.path} time: {time.perf_counter() - start}")
    time.sleep(0.2)
    inviwopy.qt.update()

canvas = network.Canvas
ivw.regression.saveCanvas(canvas, "Canvas")
