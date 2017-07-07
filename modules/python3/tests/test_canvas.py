import inviwopy as i
from inviwopy.glm import *
from ivw import utils

p = i.app.network.CanvasGL

p.widget.position = ivec2(p.widget.position[1],p.widget.position[0])
p.widget.dimensions = ivec2(200,330)

#p.widget.hide()
#p.widget.show()

print(p.widget.visibility)
p.widget.visibility = not p.widget.visibility
print(p.widget.visibility)


print( p.widget)
print( dir( p.widget))
#print(dir(p))