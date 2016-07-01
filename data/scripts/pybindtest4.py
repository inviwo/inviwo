#Inviwo Python script 
import inviwopy

help('inviwopy') 
app = inviwopy.getApp();
network = app.getProcessorNetwork();
processor = network.getProcessors()[0];

asdf = inviwopy.FloatProperty("asdasdff","qwer",0.5,0,1,0.001)

processor.addProperty(asdf)
