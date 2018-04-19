import inviwopy
from inviwopy.glm import *
from inviwopy.properties import IntVec3Property
from inviwopy.properties import IntVec3Property
from inviwopy.data import VolumeOutport
from inviwopy.data import Volume

import numpy

def init(self):
	"""
	The PythonScriptProcessor will call the init function on construction and whenever this
	script changes. Hence one needs to take care not to add ports and properties multiple times.
	The argument 'self' represents the PythonScriptProcessor.
	"""
	self.displayName = "Test Volume"
	if not hasattr(self, "outport"):
		# Need to assing the port to self, since addOutport does not take onwership
		self.outport = VolumeOutport("outport")
		self.addOutport(self.outport, "default")
	if not hasattr(self, "dim"):
		# Need to assing the property to self, since addProperty does not take onwership
		self.dim = IntVec3Property("dim", "dim", ivec3(5), ivec3(0), ivec3(20))
		self.addProperty(self.dim)

def process(self):
	"""
	The PythonScriptProcessor will call this process function whenever the processor process 
	function is called. The argument 'self' represents the PythonScriptProcessor.
	"""
	numpy.random.seed(546465)
	dim = self.dim.value;
	## create a small float volume filled with random noise
	volume = Volume(numpy.random.rand(dim[0], dim[1], dim[2]).astype(numpy.float32))
	volume.dataMap.dataRange = dvec2(0.0, 1.0)
	volume.dataMap.valueRange = dvec2(0.0, 1.0)
	self.outport.setData(volume)
