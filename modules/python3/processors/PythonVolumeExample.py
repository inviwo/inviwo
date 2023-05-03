# Name: PythonVolumeExample

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

import inviwopy as ivw
from inviwopy.properties import ConstraintBehavior as cb
import numpy as np


class PythonVolumeExample(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)

        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.addProperty(ivw.properties.IntVec3Property("dim", "Dimensions",
                                                        ivw.glm.ivec3(5),
                                                        ivw.glm.ivec3(1),
                                                        ivw.glm.ivec3(20)))

        self.addProperty(ivw.properties.IntProperty("seed", "Random Seed", 546465,
                                                    min=(0, cb.Immutable),
                                                    max=(1000000, cb.Ignore)))
        self.properties.seed.semantics = ivw.properties.PropertySemantics("Text")

        self.addProperty(ivw.properties.ButtonProperty("randomize", "Randomize Seed",
                                                       self.randomSeed,
                                                       ivw.properties.InvalidationLevel.Valid))

        self.rng = np.random.default_rng(self.properties.seed.value)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.PythonVolumeExample",
            displayName="Python Volume Example",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags("PY, Example"),
            help=ivw.md2doc(r'''
Example processor in python demonstrating the use of `inviwopy.data.Volume`
and `inviwopy.data.VolumePy`.

See [python3/pythonvolume.inv](file:~modulePath~/data/workspaces/pythonvolume.inv)
workspace for example usage.
''')
        )

    def getProcessorInfo(self):
        return PythonVolumeExample.processorInfo()

    def initializeResources(self):
        pass

    def process(self):

        dim = self.properties.dim.value

        # create a VolumePy representation
        volumerep = ivw.data.VolumePy(self.rng.random(
            (dim[2], dim[1], dim[0]), dtype=np.float32))
        # directly access volume data as numpy array via `volumerep.data`

        # create a Volume from the representation
        volume = ivw.data.Volume(volumerep)

        volume.basis = ivw.glm.mat3(1)
        volume.offset = ivw.glm.vec3(-0.5, -0.5, -0.5)

        volume.dataMap.dataRange = ivw.glm.dvec2(0.0, 1.0)
        volume.dataMap.valueRange = ivw.glm.dvec2(0.0, 1.0)

        self.outport.setData(volume)

    def randomSeed(self):
        self.properties.seed.value = self.rng.integers(2**31 - 1)
        self.rng = np.random.default_rng(seed=self.properties.seed.value)
