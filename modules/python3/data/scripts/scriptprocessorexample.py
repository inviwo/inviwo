def init(self):
	import inviwopy
	self.displayName = "Test Volume"
	if not hasattr(self, "outport"):
		self.outport = inviwopy.data.VolumeOutport("outport")
		self.addOutport(self.outport, "default")
	if not hasattr(self, "dim"):
		self.dim = inviwopy.properties.IntVec3Property("mydim", "dim", 
			inviwopy.glm.ivec3(5), 
			inviwopy.glm.ivec3(0), 
			inviwopy.glm.ivec3(20))
		self.addProperty(self.dim)

def process(self):
	import numpy
	import inviwopy
	## create a small float volume filled with random noise
	numpy.random.seed(546465)
	volume = inviwopy.data.Volume(
		numpy.random.rand(self.dim.value[0], self.dim.value[1], self.dim.value[2]).astype(numpy.float32)) 
	volume.dataMap.dataRange = inviwopy.glm.dvec2(0.0, 1.0)
	volume.dataMap.valueRange = volume.dataMap.dataRange
	self.outport.setData(volume)

