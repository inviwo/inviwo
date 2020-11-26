# Inviwo Python script 
import inviwopy
import ivw.regression
import time

network = inviwopy.app.network
key_a = inviwopy.KeyboardEvent(key = inviwopy.IvwKey.A, state = inviwopy.KeyState.Press, nativeVirtualKey = 65, utfText = 'a')
browser = network.Webbrowser
browser.invokeEvent(key_a)
key_A = inviwopy.KeyboardEvent(key = inviwopy.IvwKey.A, state = inviwopy.KeyState.Press, modifiers = inviwopy.KeyModifiers(inviwopy.KeyModifier.Shift), nativeVirtualKey = 65, utfText = 'A')
browser.invokeEvent(key_A)

# HACK: Wait for as little as possible while ensuring that the webpage has re-rendered. 
time.sleep(0.1);

canvas = network.Canvas;
ivw.regression.saveCanvas(canvas, "Canvas");

