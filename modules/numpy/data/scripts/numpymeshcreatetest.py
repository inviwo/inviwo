import numpy as np

test = 3
test2 = np.ndarray((2,2), buffer=np.array([1,2,3,4]),dtype=int) # offset = 1*itemsize, i.e. skip first element

class BasicMesh:
    def __init__(self,size):
        self.vertices  =  np.zeros((size,3),dtype=np.float32)
        self.texCoords     =  np.zeros((size,3),dtype=np.float32)
        self.normals  =  np.zeros((size,3),dtype=np.float32)
        self.colors  =  np.zeros((size,4),dtype=np.float32)

    def addTriangleList(self,np):
        print(np.dtype)
        self.triangleList = np
        print(self.triangleList.dtype)

mesh = BasicMesh(12)

def c(z):
    a = 0.75;
    b = 0.1;
    if z == -1: return (1,b,b,1)
    if z ==  0: return (b,1,b,a)
    if z ==  1: return (b,b,1,a)

idx = 0
for z in [-1,0,1]:
    for y in [-1,1]:
        for x in [-1,1]: 
            o = 0;
            if(z == -1) : 
                o = -1
            if(z == 1) : 
                o = -0.3
            mesh.vertices[idx] = [x+0.5*z,y+o+0.3,z*1.5]
            mesh.texCoords[idx] = (x,y,0.5)
            mesh.normals[idx] = (0,0,1)
            mesh.colors[idx] = c(z)
            idx += 1


ind = np.ndarray(6*3,dtype=np.uint32)

print(ind.dtype)

idx = 0
for base in [0,1,4,5,8,9]:
    for i in [0,1,2]:
        ind[idx] = base+i 
        idx+=1


print(ind.dtype)

mesh.addTriangleList(ind);


print(ind.dtype)

print("Done")
 