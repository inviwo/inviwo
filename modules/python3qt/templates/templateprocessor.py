# Name: {name} 

import inviwopy as ivw

class {name}(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.VolumeInport("inport")
        self.addInport(self.inport)
        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport)

        self.slider = ivw.properties.IntProperty("slider", "slider", 0, 0, 100, 1)
        self.addProperty(self.slider)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		"org.inviwo.{name}",     # Class identifier
    		"{name}",                # Display name
    		"Python",                # Category
    		ivw.CodeState.Stable, 	 # Code state
    		ivw.Tags("Python"),      # Tags
    		True                     # Visible   
   		)

    def getProcessorInfo(self):
        return {name}.processorInfo()

    def initializeResources(self):
        print("init")

    def process(self):
        print("process: ", self.slider.value)
        self.outport.setData(self.inport.getData())