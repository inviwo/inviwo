# Inviwo Python script 
import inviwopy
import ivw.regression
import time
import inviwopyapp as app

network = inviwopy.app.network
browser = network.Webbrowser
canvas = network.Canvas;

# HACK: Wait for as little as possible while ensuring that the webpage has rendered. 
while canvas.image.colorLayers[0].data[80, 67, 0] == 255:
    print(canvas.image.colorLayers[0].data[80, 67, 0])
    inviwopy.qt.update();
    time.sleep(0.1);
# Send key events
network.lock()
key_a = inviwopy.KeyboardEvent(key = inviwopy.IvwKey.A, state = inviwopy.KeyState.Press, nativeVirtualKey = 65, utfText = 'a')
browser.invokeEvent(key_a)
key_A = inviwopy.KeyboardEvent(key = inviwopy.IvwKey.A, state = inviwopy.KeyState.Press, modifiers = inviwopy.KeyModifiers(inviwopy.KeyModifier.Shift), nativeVirtualKey = 65, utfText = 'A')
browser.invokeEvent(key_A)
network.unlock()

ivw.regression.saveCanvas(canvas, "Canvas")

