# Name: ImageInjector

import inviwopy as ivw
from inviwopy.glm import dvec2
from inviwopy.data import InterpolationType, Layer, Image
from inviwopy.properties import InvalidationLevel

import numpy as np

def no_fp64(a): return a.astype(np.float32) if a.dtype == np.float64 else a

class ImageInjector(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.ImageOutport("outport")
        self.addOutport(self.outport, owner=False)
        self.outport.setHandleResizeEvents(False)
        self.image = None

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.ImageInjector",
    		displayName = "ImageInjector",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return ImageInjector.processorInfo()

    def initializeResources(self):
        pass

    def setArray(self, color_layers, depth_layer=None, picking_layer=None):
        layers = [Layer(no_fp64(a)) for a in color_layers] if isinstance(color_layers, list) else [Layer(no_fp64(color_layers))]
        for l in layers: l.interpolation = InterpolationType.Linear
        self.image = Image(layers[0], depth_layer, picking_layer)
        for l in layers[1:]:
            self.image.addColorLayer(l)
        if self.image.dimensions != self.outport.dimensions:
            self.outport.setDimensions(self.image.dimensions)
        self.invalidate(InvalidationLevel.InvalidOutput)

    def process(self):
        if self.image is not None:
            if self.image.dimensions != self.outport.dimensions:
                self.image.setDimensions(self.outport.dimensions)
            self.outport.setData(self.image)
