# Name: {name} 

import inviwopy as ivw

class {name}(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.ImageInport("inport")
        self.addInport(self.inport, owner=False)
        self.outport = ivw.data.ImageOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.slider = ivw.properties.IntProperty("slider", "slider", 0, 0, 100, 1)
        self.addProperty(self.slider, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.{name}", 
    		displayName = "{name}",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return {name}.processorInfo()

    def initializeResources(self):
        print("init")

    def process(self):
        print("process: ", self.slider.value)
        self.outport.setData(self.inport.getData())
