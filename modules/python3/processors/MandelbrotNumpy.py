# Name: MandelbrotNumpy

# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023-2025 Inviwo Foundation
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

import inviwopy as ivw
from inviwopy.properties import ConstraintBehavior as cb

import numpy as np
import math


class MandelbrotNumpy(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)

        self.outport = ivw.data.LayerOutport("outport")
        self.addOutport(self.outport)

        self.imgdims = ivw.properties.IntSize2Property("size", "Image Dimensions",
                                                       ivw.glm.size2_t(32, 32),
                                                       min=(ivw.glm.size2_t(
                                                           1), cb.Immutable),
                                                       max=(ivw.glm.size2_t(1024), cb.Ignore))
        self.imgdims.semantics = ivw.properties.PropertySemantics("Text")

        self.boundsReal = ivw.properties.DoubleMinMaxProperty(
            "boundsReal", "Real Bounds", valueMin=-2.0, valueMax=1.0, rangeMin=-3.0, rangeMax=3.0)
        self.boundsImaginary = ivw.properties.DoubleMinMaxProperty(
            "boundsImaginary", "Imaginery Bounds", -1.1, 1.1, -3.0, 3.0)
        self.power = ivw.properties.DoubleProperty("power", "Power", 2.0,
                                                   min=(0.0001, cb.Immutable),
                                                   max=(10.0, cb.Ignore))
        self.iterations = ivw.properties.IntProperty("iterations", "Iterations", 10,
                                                     min=(1, cb.Immutable),
                                                     max=(100, cb.Ignore))

        self.addProperties([self.imgdims, self.boundsReal,
                            self.boundsImaginary, self.power, self.iterations])

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.MandelbrotNumpy",
            displayName="Mandelbrot Numpy",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags("PY, Example"),
            help=ivw.md2doc(r'''
Example processor computing the Mandelbrot set with numpy and storing it in a
`LayerPy` representation of an `Image`.

See [python3/mandelbrot.inv](file:~modulePath~/data/workspaces/mandelbrot.inv) workspace.
''')
        )

    def getProcessorInfo(self):
        return MandelbrotNumpy.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        dims = self.imgdims.value

        realAxis = np.linspace(self.boundsReal.value.x, self.boundsReal.value.y, dims.x)
        imagineryAxis = np.linspace(self.boundsImaginary.value.x,
                                    self.boundsImaginary.value.y, dims.y)

        npData = np.zeros((dims.y, dims.x), dtype=np.float32)

        power = self.power.value
        iterations = self.iterations.value

        for index, _ in np.ndenumerate(npData):
            C = Z = complex(realAxis[index[1]], imagineryAxis[index[0]])
            for i in range(0, iterations):
                if np.abs(Z) > 2:
                    npData[index] = math.log(1 + i)
                    break
                Z = np.power(Z, power) + C

        layerpy = ivw.data.LayerPy(npData)
        layerpy.interpolation = ivw.data.InterpolationType.Nearest

        layer = ivw.data.Layer(layerpy)
        layer.dataMap.dataRange = ivw.glm.dvec2(npData.min(), npData.max())
        layer.dataMap.valueRange = layer.dataMap.dataRange

        self.outport.setData(layer)
