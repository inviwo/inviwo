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
	def __init__(self, testImage, refImage):

		self.testImage = Image.open(testImage)
		self.refImage = Image.open(refImage)

		self.diff = 100
		if self.testImage.mode == self.refImage.mode and self.testImage.size == self.refImage.size:
			ncomponents = self.testImage.size[0] * self.testImage.size[1] * len(self.testImage.getbands())
			pairs = zip(self.testImage.getdata(), self.refImage.getdata())
			if len(self.testImage.getbands()) == 1: # for gray-scale jpegs
				self.diff = sum(abs(p1-p2) for p1,p2 in pairs) * 100.0 / 255.0 / ncomponents
			else:
				self.diff = sum(abs(c1-c2) for p1,p2 in pairs for c1,c2 in zip(p1,p2)) * 100.0 / 255.0 / ncomponents


	def saveDifferenceImage(self, difffile, maskfile, enhance = 10):
		if self.testImage.mode == self.refImage.mode and self.testImage.size == self.refImage.size:
			if len(self.testImage.getbands()) == 1:
				def issame(x,y): return abs(x-y) == 0.0
			else:
				def issame(x,y): return sum([abs(i-j) for i,j in zip(x,y)]) == 0.0

			if len(self.testImage.getbands()) == 1:
				def tocolor(x,y): return (255-enhance*abs(x-y),0,0,255)
			elif len(self.testImage.getbands()) == 2:
				def tocolor(x,y): return (255-enhance*abs(x[0]-y[0]),255-enhance*abs(x[1]-y[1]), 0, 255)
			elif len(self.testImage.getbands()) >= 3:
				def tocolor(x,y): return (255-enhance*abs(x[0]-y[0]),255-enhance*abs(x[1]-y[1]),255-enhance*abs(x[2]-y[2]), 255)

			mask = Image.new('1', self.refImage.size)
			diff = Image.new('RGBA', self.refImage.size)

			d = diff.load()
			m = mask.load()

			t = self.testImage.load()
			r = self.refImage.load()

			for i in range(diff.size[0]):
				for j in range(diff.size[1]):
					m[i,j] = 1 if issame(t[i,j], r[i,j]) else 0		
					d[i,j] = tocolor(t[i,j], r[i,j])

			diff.save(difffile)
			mask.save(maskfile)


	def saveReferenceImage(self, file):
		self.refImage.save(file)

	def difference(self):
		return self.diff

	def same_size(self):
		return self.testImage.size == self.refImage.size

	def same_mode(self):
		return self.testImage.mode == self.refImage.mode
