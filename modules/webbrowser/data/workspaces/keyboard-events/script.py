# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023 Inviwo Foundation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ********************************************************************************

import inviwopy
import ivw.regression

network = inviwopy.app.network
browser = network.Webbrowser
canvas = network.Canvas

# HACK: Wait until page is loaded. Color will be set to 1
while (network.Background.bgColor2.value[0] != 1):
    inviwopy.qt.update()

# Send key events
network.lock()
key_a = inviwopy.KeyboardEvent(
    key=inviwopy.IvwKey.A, state=inviwopy.KeyState.Press, nativeVirtualKey=65, utfText='a')
browser.invokeEvent(key_a)
key_A = inviwopy.KeyboardEvent(key=inviwopy.IvwKey.A,
                               state=inviwopy.KeyState.Press,
                               modifiers=inviwopy.KeyModifiers(inviwopy.KeyModifier.Shift),
                               nativeVirtualKey=65,
                               utfText='A')
browser.invokeEvent(key_A)
network.unlock()

# HACK: Wait for as little as possible while ensuring that the webpage has rendered.
while canvas.image.colorLayers[0].data[80, 67, 0] == 255:
    print("Waiting for pixel [80, 67] to change color.")
    inviwopy.qt.update()

ivw.regression.saveCanvas(canvas, "Canvas")
