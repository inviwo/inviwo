from inviwopy.data import *
from inviwopy.glm import *
import numpy as np

mesh = BasicMesh()
 
ind = mesh.addIndexBuffer(DrawType.Triangles , ConnectivityType.None_ );
ind.size = 6*3

def c(z):
    a = 0.75;
    b = 0.1;
    if z == -1: return vec4(1,b,b,1)
    if z ==  0: return vec4(b,1,b,a)
    if z ==  1: return vec4(b,b,1,a)

for z in [-1,0,1]:
    for y in [-1,1]: 
        for x in [-1,1]: 
            o = 0;
            if(z == -1) : 
                o = -1 
            if(z == 1) : 
                o = -0.3 
            mesh.addVertex(vec3(x+0.5*z,y+o+0.3,z*1.5),vec3(0,0,1),vec3(x,y,0.5),c(z))

idx = 0
for base in [0,1,4,5,8,9]:
    for i in [0,1,2]:
        ind.data[idx] = base+i 
        idx+=1
