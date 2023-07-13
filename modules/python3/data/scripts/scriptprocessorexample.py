import inviwopy
from inviwopy.glm import ivec3, dvec2
from inviwopy.properties import IntVec3Property
from inviwopy.data import VolumeOutport, VolumeInport
from inviwopy.data import Volume
import numpy

"""
The PythonScriptProcessor will run this script on construction and whenever this
it changes. Hence one needs to take care not to add ports and properties multiple times.
The PythonScriptProcessor is exposed as the local variable 'self'.
"""

#if not "dim" in self.properties:
#	self.addProperty(IntVec3Property("dim", "dim", ivec3(5), ivec3(0), ivec3(20)))

if not "outport" in self.outports:
	self.addOutport(VolumeOutport("outport"))
if not "inport" in self.inports:
	self.addInport(VolumeInport("inport"))

def process(self):
	#volume.dataMap.dataRange = dvec2(0.0, 1.0)
    #volume.dataMap.valueRange = dvec2(0.0, 1.0)
    if not "inport" in self.inports or not "outport" in self.outports or not self.inports.inport.hasData():
        pass
    self.outports.outport.setData(self.inports.inport.getData())

def initializeResources(self):
	pass

# Tell the PythonScriptProcessor about the 'initializeResources' function we want to use
self.setInitializeResources(initializeResources)

# Tell the PythonScriptProcessor about the 'process' function we want to use
self.setProcess(process)