# Inviwo Python script
import inviwopy
import ivw.regression
import time

m = ivw.regression.Measurements()
start = time.perf_counter()

network = inviwopy.app.network

# HACK: Wait for as little as possible while ensuring that the webpage has re-rendered.
isLoading = network.Webbrowser.isLoading
inviwopy.logInfo(f"{isLoading.path}: {isLoading.value}")
while isLoading.value:
    time.sleep(0.2)
    inviwopy.qt.update()
    inviwopy.logInfo(f"Waiting for webpage {isLoading.path}: {isLoading.value}"
                     f" time: {time.perf_counter() - start}")

# Trigger an extra network evaluation to try and make sure the web suff is done
VolumeCreator.invalidate(inviwopy.properties.InvalidationLevel.InvalidOutput)

canvas = network.Canvas
ivw.regression.saveCanvas(canvas, "Canvas")

end = time.perf_counter()
m.addTime("Total Time", end - start)
m.save()
