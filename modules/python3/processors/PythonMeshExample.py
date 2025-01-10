# Name: PythonMeshExample

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

import numpy as np


def ngonLineMesh(n: int, tf: ivw.data.TransferFunction):
    """
    create the outline of an n-gon using inviwopy.data.Mesh and adding individual buffers for
    positions, colors, and indices
    """

    # create a standard Inviwo mesh
    mesh = ivw.data.Mesh(dt=ivw.data.DrawType.Lines, ct=ivw.data.ConnectivityType.Loop)

    # set the model transformation of the mesh
    mesh.modelMatrix = ivw.glm.mat4(1)
    # alternatively, the model transformation can also be modified
    # with `mesh.basis`and `mesh.offset`
    # mesh.basis = ivw.glm.mat3(1)
    # mesh.offset = ivw.glm.vec3(0, 0, 0)

    angles = np.arange(n, dtype=np.float32) * np.pi * 2.0 / n

    positions = np.stack([np.cos(angles), np.sin(angles)], axis=1)
    colors = np.array([tf.sample(x) for x in np.arange(n) / n], dtype=np.float32)

    mesh.addBuffer(ivw.data.BufferType.PositionAttrib, ivw.data.Buffer(positions))
    mesh.addBuffer(ivw.data.BufferType.ColorAttrib, ivw.data.Buffer(colors))

    mesh.addIndices(ivw.data.MeshInfo(dt=ivw.data.DrawType.Lines,
                                      ct=ivw.data.ConnectivityType.Loop),
                    ivw.data.IndexBufferUINT32(np.arange(n, dtype=np.uint32)))

    return mesh


def ngonTriangleMesh(n: int, tf: ivw.data.TransferFunction):
    """
    create an n-gon using inviwopy.data.BasicMesh which comes with predefined buffers.

    BasicMesh holds the following buffers:
    - buffertraits::PositionsBuffer     (ivw.glm.vec3)
    - buffertraits::NormalBuffer        (ivw.glm.vec3)
    - buffertraits::TexCoordBuffer<3>   (ivw.glm.vec3)
    - buffertraits::ColorsBuffer        (ivw.glm.vec4)
    """

    mesh = ivw.data.BasicMesh(dt=ivw.data.DrawType.Triangles, ct=ivw.data.ConnectivityType.Fan)

    normal = ivw.glm.vec3(0, 0, 1)
    # add mid point
    mesh.addVertex(ivw.glm.vec3(0, 0, 0), normal, ivw.glm.vec3(0, 0, 0), ivw.glm.vec4(1, 1, 1, 1))

    angles = np.arange(n, dtype=np.float32) * np.pi * 2.0 / n
    angles = np.append(angles, [0])

    # create a list of individual vertices where each vertex is a tuple
    # of position, normal, texcoord, and color
    vertexdata = [(
        ivw.glm.vec3(np.cos(angle), np.sin(angle), 0),  # position
        normal,                                         # normal
        ivw.glm.vec3(1.0, angle / (2.0 * np.pi), 0),    # texture coordinate
        tf.sample(angle / (2.0 * np.pi))                # color
    ) for angle in angles]

    mesh.addVertices(vertexdata)

    return mesh


class PythonMeshExample(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.addOutport(ivw.data.MeshOutport("linemesh"))
        self.addOutport(ivw.data.MeshOutport("trianglemesh"))

        self.sides = ivw.properties.IntProperty("sides", "Sides", 6, 2, 32, 1)
        self.addProperty(self.sides, owner=False)

        self.tf = ivw.properties.TransferFunctionProperty("tf", "Transfer Function")
        self.addProperty(self.tf, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.PythonMeshExample",
            displayName="Python Mesh Example",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags("PY, Example"),
            help=ivw.md2doc(r'''
Example processor in python demonstrating the use of `inviwopy.data.Mesh`
and `inviwopy.data.BasicMesh`.

See [python3/pythonmeshes.inv](file:~modulePath~/data/workspaces/pythonmeshes.inv)
workspace for example usage.
''')
        )

    def getProcessorInfo(self):
        return PythonMeshExample.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        linemesh = ngonLineMesh(n=self.sides.value, tf=self.tf.value)
        self.outports.linemesh.setData(linemesh)

        mesh = ngonTriangleMesh(n=self.sides.value, tf=self.tf.value)
        self.outports.trianglemesh.setData(mesh)
