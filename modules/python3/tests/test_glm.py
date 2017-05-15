#Inviwo Python script 
import inviwopy
from inviwopy.glm import *





m1 = mat4(1)

print(m1)

m2 = mat3(0,1,0,1,0,0,0,0,2)

print(m2)

v2 = vec3(1,2,3)
v3 = m2 * v2

print (v2)
print (v3)

print(v3.x)
print(v3.y)
print(v3.z)

