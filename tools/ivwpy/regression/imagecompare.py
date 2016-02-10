#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2015 Inviwo Foundation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
#*********************************************************************************

import PIL.Image as Image

class ImageCompare:
	def __init__(self, testImage, refImage, enhance = 10):

		self.testImage = Image.open(testImage)
		self.refImage = Image.open(refImage)

		self.maskImage = None
		self.diffImage = None
		self.difference = 100
		self.numberOfDifferentPixels = None
		self.maxDifference = None

		if self.testImage.mode != self.refImage.mode or self.testImage.size != self.refImage.size:
			return 

		if len(self.testImage.getbands()) == 1: # for gray-scale jpegs
			def norm(x,y): return abs(x-y) / 255.0
		else:
			def norm(x,y): 
				return sum([abs(i-j) for i,j in zip(x,y)]) / \
						(len(self.testImage.getbands()) * 255.0)

		def issame(x,y): return norm(x, y) == 0.0

		if len(self.testImage.getbands()) == 1:
			def tocolor(x,y): 
				return (255-enhance*abs(x-y),
				        255-enhance*abs(x-y),
				        255-enhance*abs(x-y),
				        255)

		elif len(self.testImage.getbands()) == 2:
			def tocolor(x,y): 
				return (255-enhance*abs(x[0]-y[0]),
					    255-enhance*abs(x[1]-y[1]),
					    255, 
					    255)

		elif len(self.testImage.getbands()) >= 3:
			def tocolor(x,y): 
				return (255-enhance*abs(x[0]-y[0]),
					    255-enhance*abs(x[1]-y[1]),
					    255-enhance*abs(x[2]-y[2]), 
					    255)

		self.maskImage = Image.new('1', self.refImage.size)
		self.diffImage = Image.new('RGBA', self.refImage.size)

		d = self.diffImage.load()
		m = self.maskImage.load()
		t = self.testImage.load()
		r = self.refImage.load()

		self.numberOfDifferentPixels = 0
		self.maxDifference = 0
		self.difference = 0

		for i in range(self.diffImage.size[0]):
			for j in range(self.diffImage.size[1]):
				n = norm(t[i,j], r[i,j])
				self.maxDifference = max(self.maxDifference, n)
				self.difference += n
				d[i,j] = tocolor(t[i,j], r[i,j])

				if n == 0.0:
					m[i,j] = 1
				else:
					m[i,j] = 0
					self.numberOfDifferentPixels += 1

		pixels = self.testImage.size[0] * self.testImage.size[1]
		self.difference = self.difference * 100.0 / pixels

	def saveDifferenceImage(self, difffile):
		if self.diffImage is not None: 
			self.diffImage.save(difffile)
			return True
		else: 
			return False
			
	def saveMaskImage(self, maskfile):
		if self.maskImage is not None: 
			self.maskImage.save(maskfile)
			return True
		else: 
			return False

	def saveReferenceImage(self, file):
		self.refImage.save(file)

	def getRefSize(self):
		return self.refImage.size
	def getRefMode(self):
		return self.refImage.mode

	def getTestSize(self):
		return self.testImage.size
	def getTestMode(self):
		return self.testImage.mode

	def getDifference(self):
		return self.difference
	
	def getNumberOfDifferentPixels(self):
		return self.numberOfDifferentPixels
	
	def getMaxDifference(self):
		return self.maxDifference

	def isSameSize(self):
		return self.testImage.size == self.refImage.size

	def isSameMode(self):
		return self.testImage.mode == self.refImage.mode
