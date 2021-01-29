# Name: VolumeInjector

import inviwopy as ivw
from inviwopy.glm import dvec2
from inviwopy.data import InterpolationType
from inviwopy.properties import InvalidationLevel

import numpy as np

class VolumeInjector(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport, owner=False)
        self.array = None

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.VolumeInjector",
    		displayName = "VolumeInjector",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return VolumeInjector.processorInfo()

    def initializeResources(self):
        pass

    def setArray(self, array):
        if array.dtype == np.float64:
            array = array.astype(np.float32)
        self.array = array
        self.invalidate(InvalidationLevel.InvalidOutput)

    def process(self):
        if self.array is not None:
            vol = Volume(np.asfortranarray(self.array))
            vol.interpolation = InterpolationType.Linear
            vol.dataMap.dataRange, vol.dataMap.valueRange = dvec2(0.0, 1.0), dvec2(0.0, 1.0)
            self.outport.setData(vol)
