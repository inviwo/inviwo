# Inviwo Python script 
import inviwopy
from inviwopy.glm import vec2,vec3

def color(x):
   r = x
   g = 1-x
   return vec3(r,g,0)

tf = inviwopy.app.network.VolumeRaycaster.transferFunction

tf.clear()
tf.addPoint(vec2(0.0,0.0),vec3(0,0,0)) 

for i in range(1,256,9):
   x = i / 256.0
   a = 0
   if i%2==1:
      a = 0.1
   tf.addPoint(vec2(x,a),color(x)) 


tf.addPoint(vec2(1.0,0.0),vec3(0,0,0)) 
