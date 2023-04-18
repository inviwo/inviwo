# Name: MandelbrotNumpy

import inviwopy as ivw

import numpy as np
import math

class MandelbrotNumpy(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)

        self.outport = ivw.data.ImageOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.imgdims = ivw.properties.IntSize2Property("size", "Image Dimensions",
            ivw.glm.size2_t(32, 2),
            min=(ivw.glm.size2_t(1), ivw.properties.ConstraintBehavior.Immutable),
            max=(ivw.glm.size2_t(1024), ivw.properties.ConstraintBehavior.Ignore))
        self.imgdims.semantics = ivw.properties.PropertySemantics("Text")

        self.boundsReal = ivw.properties.DoubleMinMaxProperty("boundsReal",
            "Real Bounds", valueMin=-2.0, valueMax=1.0, rangeMin=-3.0, rangeMax=3.0)
        self.boundsImaginary = ivw.properties.DoubleMinMaxProperty("boundsImaginary",
            "Imaginery Bounds", -1.1, 1.1, -3.0, 3.0)
        self.power = ivw.properties.DoubleProperty("power", "Power", 2.0,
            min=(0.0001, ivw.properties.ConstraintBehavior.Immutable),
            max=(10.0, ivw.properties.ConstraintBehavior.Ignore))
        self.iterations = ivw.properties.IntProperty("iterations", "Iterations", 10,
            min=(1, ivw.properties.ConstraintBehavior.Immutable),
            max=(100, ivw.properties.ConstraintBehavior.Ignore))

        self.addProperties([self.imgdims, self.boundsReal,
            self.boundsImaginary, self.power, self.iterations])

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.MandelbrotNumpy",
            displayName="Mandelbrot Numpy",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags("PY, Example"),
            help=ivw.md2doc(r'''
Example processor computing the Mandelbrot set with numpy and storing it in a
`LayerPy` representation of an `Image`.

See [python3/mandelbrot.inv](file:~modulePath~/data/workspaces/mandelbrot.inv) workspace.
''')
        )

    def getProcessorInfo(self):
        return MandelbrotNumpy.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        dims = self.imgdims.value

        realAxis = np.linspace(self.boundsReal.value.x, self.boundsReal.value.y, dims.x)
        imagineryAxis = np.linspace(self.boundsImaginary.value.x, self.boundsImaginary.value.y, dims.y)

        npData = np.zeros((dims.y, dims.x), dtype=np.float32)

        power = self.power.value
        iterations = self.iterations.value

        for index, _ in np.ndenumerate(npData):
            C = Z = complex(realAxis[index[1]], imagineryAxis[index[0]])
            for i in range(0, iterations):
                if np.abs(Z) > 2:
                    npData[index] = math.log(1 + i)
                    break
                Z = np.power(Z, power) + C

        npData = npData.reshape((npData.shape[1], npData.shape[0]))

        layerpy = ivw.data.LayerPy(npData)
        layerpy.interpolation = ivw.data.InterpolationType.Nearest

        self.outport.setData(ivw.data.Image(ivw.data.Layer(layerpy)))
