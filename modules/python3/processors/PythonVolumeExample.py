# Name: PythonVolumeExample

import inviwopy as ivw
import numpy

class PythonVolumeExample(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)

        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.addProperty(ivw.properties.IntVec3Property("dim", "Dimensions", 
            ivw.glm.ivec3(5), ivw.glm.ivec3(1), ivw.glm.ivec3(20)))

        self.addProperty(ivw.properties.IntProperty("seed", "Random Seed", 546465,
            min=(0, ivw.properties.ConstraintBehavior.Immutable),
            max=(1000000, ivw.properties.ConstraintBehavior.Ignore)))
        self.properties.seed.semantics = ivw.properties.PropertySemantics("Text")

        self.addProperty(ivw.properties.ButtonProperty("randomize", "Randomize Seed",
            self.randomSeed, ivw.properties.InvalidationLevel.Valid))

        self.rng = numpy.random.default_rng(self.properties.seed.value)

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

See [python3/pythonvolume.inv](file:~modulePath~/data/workspaces/pythonvolume.inv) workspace for example usage.
''')
        )

    def getProcessorInfo(self):
        return PythonVolumeExample.processorInfo()

    def initializeResources(self):
        pass

    def process(self):

        dim = self.properties.dim.value

        # create a VolumePy representation
        volumerep = ivw.data.VolumePy(self.rng.random((dim[0], dim[1], dim[2]), dtype=numpy.float32))
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
        self.rng = numpy.random.default_rng(seed=self.properties.seed.value)
