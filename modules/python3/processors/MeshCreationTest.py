# Name: MeshCreationTest

import inviwopy as ivw
import numpy
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
            indices.extend([ index, index + 1, index + 2, index + 2, index + 1, index + 3 ])
            index += 4

        mesh.addIndices(ivw.data.MeshInfo(dt=ivw.data.DrawType.Triangles, ct=ivw.data.ConnectivityType.Unconnected),
            ivw.data.IndexBufferUINT32(numpy.array(indices, dtype=np.uint32)))
        
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
