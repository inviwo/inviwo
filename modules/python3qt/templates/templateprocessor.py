# Name: {name}

import inviwopy as ivw


class {name}(ivw.Processor):
    """
    Documentation of {name}
    """

    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.ImageInport("inport")
        self.addInport(self.inport)
        self.outport = ivw.data.ImageOutport("outport")
        self.addOutport(self.outport)

        cb = ivw.properties.ConstraintBehavior
        self.slider = ivw.properties.IntProperty(
            "slider", "Slider",
            help=ivw.md2doc("Basic slider property"),
            value=0, increment=1, min=(0, cb.Ignore), max=(100, cb.Ignore))
        self.addProperties([self.slider])

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.{name}",
            displayName="{name}",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.unindentMd2doc({name}.__doc__)
        )

    def getProcessorInfo(self):
        return {name}.processorInfo()

    def initializeResources(self):
        print("init")

    def process(self):
        print("process: ", self.slider.value)
        self.outport.setData(self.inport.getData())
