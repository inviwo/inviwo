from inviwopy.data import *
from inviwopy.glm import *
from inviwopy.data.formats import *
import inviwopy

import numpy

## the PythonScriptProcessor will fetch data from the "mesh" object and the "volume" object

## create a  mesh
mesh = BasicMesh()
 

mesh.addVertex(vec3(-0.2, -0.2, -0.1), vec3(0, 0, 1), vec3(0, 0, 0), vec4(0.1, 0.1, 0.1, 0.9))
mesh.addVertex(vec3(0.75, -0.2, -0.1), vec3(0, 0, 1), vec3(0, 0, 0), vec4(0.6, 0.1, 0.1, 0.9))
mesh.addVertex(vec3(0.75, 0.75, -0.1), vec3(0, 0, 1), vec3(0, 0, 0), vec4(0.1, 0.6, 0.1, 0.9))
mesh.addVertex(vec3(-0.2, 0.75, -0.1), vec3(0, 0, 1), vec3(0, 0, 0), vec4(0.1, 0.1, 0.6, 0.9))

## create matching index buffer
ind = mesh.addIndexBuffer(DrawType.Triangles , ConnectivityType.None_ );
ind.size = 6
idx = 0
for i in [0, 1, 2, 0, 2, 3] :
    ind.data[idx] = i
    idx += 1



## create a small float volume filled with random noise
dim = size3_t(12, 12, 12)

volume = Volume(dim, DataFLOAT32) 
volume.dataMap.dataRange = dvec2(0.0, 1.0)
volume.dataMap.valueRange = volume.dataMap.dataRange

numpy.random.seed(546465)
data = numpy.random.rand(dim[0], dim[1], dim[2]).astype(numpy.float32)

volume.setData(data)
