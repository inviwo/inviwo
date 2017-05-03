#Inviwo Python script 
import inviwopy
from inviwopy import qt
from inviwopy.glm import ivec2

app = inviwopy.app
network = app.network
factory = app.processorFactory;

if app.network.i == None:
    p = factory.create('org.inviwo.IntSliderTest',  ivec2(75 , -100))
    p.identifier = "i"
    network.addProcessor(p)

    
if app.network.f == None:
    p = factory.create('org.inviwo.FloatSliderTest',  ivec2(75 , -25))
    p.identifier = "f"
    network.addProcessor(p)

app.network.i.prop1.minValue = -10
app.network.i.prop1.maxValue = 10

app.network.f.prop1.minValue = -1
app.network.f.prop1.maxValue = 1
app.network.f.prop1.increment = 0.2

s = ""
for i in app.network.i.prop1.foreach(-3,8):
        s += str(i) + " ";
        qt.update();
print(s)


s = ""
for i in app.network.i.prop1.foreach(-3,8,2):
        s += str(i) + " ";
        qt.update();
print(s)



for p in [app.network.i.prop1 , app.network.f.prop1 ]:
    s = ""
    for i in p:
        s += str(i) + " ";
        qt.update();
    print(s)