# Name: MeshInjector

import inviwopy as ivw
from inviwopy.glm import dvec2
from inviwopy.data import InterpolationType, Buffer, Mesh, DrawType, ConnectivityType, BufferType
from inviwopy.properties import InvalidationLevel

import numpy as np

class MeshInjector(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.MeshOutport("outport")
        self.addOutport(self.outport, owner=False)
        self.array = None

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.MeshInjector",
    		displayName = "MeshInjector",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return MeshInjector.processorInfo()

    def initializeResources(self):
        pass

    # def setArray(self, draw_type=DrawType.NotSpecified, connectivity_type=ConnectivityType.None, **buffers):
    #     self.mesh = Mesh(draw_type, connectivity_type)
    #     for bt, array in buffers:
    #         if array.dtype == np.float64:
    #             array = array.astype(np.float32)
    #         self.mesh.addBuffer(Buffer(array), BufferType(bt))
    #     self.invalidate(InvalidationLevel.InvalidOutput)

    def process(self):
        if self.array is not None:
            vol = Mesh(Buffer(np.asfortranarray(self.array)))
            vol.interpolation = InterpolationType.Linear
            vol.dataMap.dataRange, vol.dataMap.valueRange = dvec2(0.0, 1.0), dvec2(0.0, 1.0)
            self.outport.setData(vol)
