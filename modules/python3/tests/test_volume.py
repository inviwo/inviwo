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
from inviwopy.glm import *


app = inviwopy.app
network = app.network

network.clear()

volProcessor = app.processorFactory.create('org.inviwo.VectorFieldGenerator3D', ivec2(0, 0))
infoProcessor = app.processorFactory.create('org.inviwo.VolumeInformation', ivec2(0, 100))

network.addProcessor(volProcessor)
network.addProcessor(infoProcessor)

network.addConnection(volProcessor.outports[0], infoProcessor.inports[0])


volProcessor.xRange.range = vec2(0, 1)
volProcessor.yRange.range = vec2(0, 1)
volProcessor.zRange.range = vec2(0, 1)
volProcessor.x.value = 'x*15'
volProcessor.y.value = 'y*15'
volProcessor.z.value = 'z*15'

volPort = volProcessor.outports[0]
volume = volPort.data


print(volume.dimensions)
data = volume.data

print(data[3, 4, 5])
