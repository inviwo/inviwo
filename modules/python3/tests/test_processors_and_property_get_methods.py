#Inviwo Python script 
import inviwopy

help('inviwopy') 
app = inviwopy.getApp();
network = app.getProcessorNetwork();
processor = network.getProcessors()[0];
prop = processor.getProperties()[0];


print(processor.getClassIdentifier())
print(processor.getDisplayName())
print(processor.getCategory())
print(processor.setIdentifier("testing asdf"))
print(processor.getIdentifier())
print(processor.hasProcessorWidget())
print(processor.getNetwork())
print(processor.hasProcessorWidget())
print(processor.getPath())

print(prop.setIdentifier("setIdentifier"))
print(prop.getIdentifier())
print(prop.getPath())
print(prop.setDisplayName("setDisplayName"))
print(prop.getDisplayName())
print(prop.getClassIdentifierForWidget())
print(prop.setReadOnly(True))
print(prop.getReadOnly())
print(prop.updateWidgets())
print(prop.setCurrentStateAsDefault())
print(prop.resetToDefaultState())
print(prop.get())

prop.set( 0.51 )
print(prop.get())

print(type(prop))

#inviwopy.snapshotCanvas("Background","D:/test.png")