# Name: VolumeCreationTest

import inviwopy as ivw
import math
import numpy
import itertools

class VolumeCreationTest(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)

        self.outport = ivw.data.VolumeOutport("volume", ivw.md2doc("Signed distance volume"))
        self.addOutport(self.outport, owner=False)

        self.addProperty(ivw.properties.IntSize3Property("size", "Dimensions",
            ivw.glm.size3_t(64),
            min=(ivw.glm.size3_t(2), ivw.properties.ConstraintBehavior.Immutable),
            max=(ivw.glm.size3_t(512), ivw.properties.ConstraintBehavior.Ignore)))

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.VolumeCreationTest",
            displayName="Volume Creation Test",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.md2doc(r'''
En example processor illustrating how Python can be used to create volumes.
This processor computes a signed distance function for each voxel.

See also [python3/processors/PythonVolumeExample.py](file:///~modulePath~/processors/PythonVolumeExample.py).
''')
        )

    def getProcessorInfo(self):
        return VolumeCreationTest.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        dim = self.properties.size.value

        npData = numpy.ndarray((dim.z, dim.y, dim.x), dtype=np.float32)

        def normalizeDeviceCoords(voxel: tuple):
            return (voxel[0] / (dim[0] - 1) * 2.0 - 1.0, 
                voxel[1] / (dim[1] - 1) * 2.0 - 1.0, 
                voxel[2] / (dim[2] - 1) * 2.0 - 1.0, )

        def len2(x, y):
            return math.sqrt(x * x + y * y)
        def len3(p: tuple):
            return math.sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2])
        def dist(coord: tuple):
            d = min(len2(coord[0], coord[1]), 
                min(len2(coord[1], coord[2]) * 0.8, 
                    min(len2(coord[0], coord[2]) * 0.6, 
                        len3(coord) * 0.2)))
            return 1.0 - d

        # initialize each voxel by applying the distance function to its voxel index
        for x, y, z in itertools.product(range(0, dim[0]), range(0, dim[1]), range(0, dim[2])):
            npData[z, y, x] = dist(normalizeDeviceCoords((x, y, z)))
        
        # the same with numpy's array iterator
        with np.nditer(npData, flags=['multi_index'], op_flags=['writeonly']) as it:
            for elem in it:
                # flip multi_index to obtain (x,y,z)
                elem = dist(np.flip(it.multi_index))

        # create a VolumePy representation
        volumerep = ivw.data.VolumePy(npData)

        # create a Volume from the representation
        volume = ivw.data.Volume(volumerep)
        volume.basis = ivw.glm.mat3(1)
        volume.offset = ivw.glm.vec3(-0.5, -0.5, -0.5)

        volume.dataMap.dataRange = ivw.glm.dvec2(0.0, npData.max())
        volume.dataMap.valueRange = ivw.glm.dvec2(0.0, npData.max())

        self.outport.setData(volume)
