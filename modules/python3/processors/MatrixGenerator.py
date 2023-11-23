# Name: MatrixGenerator

import inviwopy as ivw
from inviwopy.properties import ConstraintBehavior as cb
import numpy as np


class MatrixGenerator(ivw.Processor):
    """
    Create a list of random mat2/mat3/mat4 matrices
    The number of items generated is equal to the product of the three grid values.
    """

    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
    
        self.dim = ivw.properties.IntProperty("dim", "Matrix dim", 3, 2, 4)
        self.addProperty(self.dim, owner=False)

        self.outport = ivw.mat3VectorOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.slider = ivw.properties.IntVec3Property("grid", "grid", 
            ivw.glm.ivec3(5), ivw.glm.ivec3(0), ivw.glm.ivec3(100))
        self.addProperty(self.slider, owner=False)

        self.range = ivw.properties.FloatVec2Property("range", "range", 
            ivw.glm.vec2(-1.0,1.0), ivw.glm.vec2(-100), ivw.glm.vec2(100))
        self.addProperty(self.range, owner=False)

        self.addProperty(ivw.properties.IntProperty("seed", "Random Seed", 546465,
                                                    min=(0, cb.Immutable),
                                                    max=(1000000, cb.Ignore)))
        self.properties.seed.semantics = ivw.properties.PropertySemantics("Text")

        self.rng = np.random.default_rng(self.properties.seed.value)

        self.dim.onChange(self.updateDim)

    def updateDim(self):
        if self.dim.value == 2:
            self.removeOutport(self.outport)
            self.outport = ivw.mat2VectorOutport("outport")
            self.addOutport(self.outport, owner=False)
        elif self.dim.value == 3:
            self.removeOutport(self.outport)
            self.outport = ivw.mat3VectorOutport("outport")
            self.addOutport(self.outport, owner=False)
        elif self.dim.value == 4:
            self.removeOutport(self.outport)
            self.outport = ivw.mat4VectorOutport("outport")
            self.addOutport(self.outport, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.MatrixGenerator",
            displayName="Matrix Generator",
            category="Data Creation",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.unindentMd2doc(MatrixGenerator.__doc__)
        )

    def getProcessorInfo(self):
        return MatrixGenerator.processorInfo()

    def process(self):
        size = self.slider.value[0]*self.slider.value[1]*self.slider.value[2]
        dataRange = self.range.value[1]-self.range.value[0]
        dataMin = self.range.value[0]
        dim = self.dim.value
        data = dataRange * (self.rng.random((size, dim, dim), dtype=np.float32) + 0.0001) + dataMin

        if dim == 2:
            self.outport.setData(inviwopy.glm.mat2Vector(data))
        elif dim == 3:
            self.outport.setData(inviwopy.glm.mat3Vector(data))
        elif dim == 4:
            self.outport.setData(inviwopy.glm.mat4Vector(data))
