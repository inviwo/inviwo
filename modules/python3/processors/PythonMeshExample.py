# Name: PythonMeshExample

import inviwopy as ivw

import numpy


def ngonLineMesh(n: int, tf: ivw.data.TransferFunction):
    """
    create the outline of an n-gon using inviwopy.data.Mesh and adding individual buffers for
    positions, colors, and indices
    """

    # create a standard Inviwo mesh
    mesh = ivw.data.Mesh(dt=ivw.data.DrawType.Lines, ct=ivw.data.ConnectivityType.Loop)

    # set the model transformation of the mesh
    mesh.modelMatrix = ivw.glm.mat4(1)
    # alternatively, the model transformation can also be modified with `mesh.basis`and `mesh.offset`
    # mesh.basis = ivw.glm.mat3(1)
    # mesh.offset = ivw.glm.vec3(0, 0, 0)

    angles = np.arange(n, dtype=np.float32) * np.pi * 2.0 / n

    positions = np.stack([np.cos(angles), np.sin(angles)], axis=1)
    colors = np.array([ tf.sample(x) for x in np.arange(n) / n], dtype=np.float32)

    mesh.addBuffer(ivw.data.BufferType.PositionAttrib, ivw.data.Buffer(positions))
    mesh.addBuffer(ivw.data.BufferType.ColorAttrib, ivw.data.Buffer(colors))

    mesh.addIndices(ivw.data.MeshInfo(dt=ivw.data.DrawType.Lines, ct=ivw.data.ConnectivityType.Loop),
        ivw.data.IndexBufferUINT32(numpy.arange(n, dtype=np.uint32)))

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
    vertexdata = [ (
            ivw.glm.vec3(np.cos(angle), np.sin(angle), 0),  # position
            normal,                                         # normal
            ivw.glm.vec3(1.0, angle / (2.0 * np.pi), 0),    # texture coordinate
            tf.sample(angle / (2.0 * np.pi))                # color
        ) for angle in angles ]

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

See [python3/pythonmeshes.inv](file:~modulePath~/data/workspaces/pythonmeshes.inv) workspace for example usage.
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
