# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023-2024 Inviwo Foundation
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
import inviwopy.glm as glm

app = inviwopy.app
network = app.network

app.network.clear()
app.network.lock()

bg = app.processorFactory.create("org.inviwo.Background", glm.ivec2(0, -100))
bg.backgroundStyle.selectedDisplayName = "Uniform color"
bg.bgColor1.value = glm.vec4(1, 1, 1, 1)

canvas = app.processorFactory.create("org.inviwo.CanvasGL", glm.ivec2(0, 0))
canvas.inputSize.dimensions.value = glm.size2_t(256, 600)

network.addProcessor(bg)
network.addProcessor(canvas)

proc = app.processorFactory.create("org.inviwo.TextOverlayGL")

fontNames = proc.font.fontFace.displayNames
fontIdentifiers = proc.font.fontFace.identifiers

prev = bg

fontSize = 16
processorSpacing = 50
# vertical spacing of text
vertTextSpacing = 1.0 / len(fontNames)

# create one text overlay processor for each font face
for i, (name, id) in enumerate(zip(fontNames, fontIdentifiers)):
    p = app.processorFactory.create("org.inviwo.TextOverlayGL",
                                    glm.ivec2(300, processorSpacing * i))
    p.displayName = name
    p.font.color.value = glm.vec4(0, 0, 0, 1)
    p.font.fontFace.selectedIdentifier = id
    p.font.fontSize.value = fontSize
    p.font.anchor.value = glm.vec2(-1, 1)

    p.texts.text0.text.value = name
    p.texts.text0.position.value = glm.vec2(0.01, 1.0 - i * vertTextSpacing)

    network.addProcessor(p)

    if i > 0:
        # link font size
        network.addLink(prev.font.fontSize, p.font.fontSize)
        network.addLink(p.font.fontSize, prev.font.fontSize)

    network.addConnection(prev.outports[0], p.inports[0])
    prev = p

canvas.position.value = glm.ivec2(0, processorSpacing * len(fontNames) + 25)

# connect last processor to canvas
network.addConnection(prev.outports[0], canvas.inports[0])

app.network.unlock()
