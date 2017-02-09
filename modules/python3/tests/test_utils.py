import inviwopy as i
from inviwopy.glm import *
from ivw import utils

p = i.app.network.CanvasGL


#for prop in p.properties:
#    print(prop)

print(i.app.network.processors)
print(type(i.app.network.processors))


print(p)
print(dir(p))
print(type(p))


print(p.inputSize.dimensions)
print(type(p.inputSize.dimensions))

#print(i.app.network.processors[2])

for pp in i.app.network.processors:
        print([type(pp),pp])


p.widget.position += ivec2(10,10)

print( p.widget)
print( dir( p.widget))
#print(dir(p))