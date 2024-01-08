# Name: MeshCreationTest

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

import inviwopy as ivw
import numpy as np
import itertools


class MeshCreationTest(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.MeshOutport("outport")
        self.addOutport(self.outport, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.MeshCreationTest",
            displayName="Mesh Creation Test",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.md2doc(r'''
Creating a `BasicMesh` of four quads in Python.

See also [python3/processors/PythonMeshExample.py](file:///~modulePath~/processors/PythonMeshExample.py).
''')
        )

    def getProcessorInfo(self):
        return MeshCreationTest.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        mesh = ivw.data.BasicMesh(dt=ivw.data.DrawType.Triangles,
                                  ct=ivw.data.ConnectivityType.Unconnected)

        for z, y, x in itertools.product([-1, 0, 1], [-1, 1], [-1, 1]):
            offset = 0
            if(z < 0):
                offset = -1
            elif(z > 0):
                offset = -0.3
            mesh.addVertex(ivw.glm.vec3(x + 0.5 * z, y + offset + 0.3, z * 1.5),
                           ivw.glm.vec3(0, 0, 1), ivw.glm.vec3(x, y, 0.5), self.color(z))

        # generate indices for 3 quads
        index = 0
        indices = []
        for _ in range(3):
            indices.extend([index, index + 1, index + 2, index + 2, index + 1, index + 3])
            index += 4

        mesh.addIndices(ivw.data.MeshInfo(dt=ivw.data.DrawType.Triangles,
                                          ct=ivw.data.ConnectivityType.Unconnected),
                        ivw.data.IndexBufferUINT32(np.array(indices, dtype=np.uint32)))

        self.outport.setData(mesh)

    def color(self, depth):
        a = 0.75
        b = 0.1
        if depth < 0:
            return ivw.glm.vec4(1, b, b, 1)
        elif depth > 0:
            return ivw.glm.vec4(b, b, 1, a)
        else:
            return ivw.glm.vec4(b, 1, b, a)
