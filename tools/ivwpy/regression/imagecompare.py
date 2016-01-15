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
import PIL.ImageChops as ImageChops
import PIL.ImageDraw as ImageDraw


class ImageCompare:
	def __init__(self, img1, img2):

		self.image1 = Image.open(img1)
		self.image2 = Image.open(img2)

		self.diff = 100
		if self.image1.mode == self.image2.mode and self.image1.size == self.image2.size:
			ncomponents = self.image1.size[0] * self.image1.size[1] * 3
			pairs = zip(self.image1.getdata(), self.image2.getdata())
			if len(self.image1.getbands()) == 1: # for gray-scale jpegs
				self.diff = sum(abs(p1-p2) for p1,p2 in pairs) * 100.0 / 255.0 / ncomponents
			else:
				self.diff = sum(abs(c1-c2) for p1,p2 in pairs for c1,c2 in zip(p1,p2)) * 100.0 / 255.0 / ncomponents


	def saveDifferenceImage(self, file, showBox = True):
		diffimg = ImageChops.difference(self.image1, self.image2)
		if showBox:
			imageDraw = ImageDraw.Draw(diffimg)
			imageDraw.rectangle(diffimg.getbbox(), outline = "red")

		diffimg.save(file)

	def difference(self):
		return self.diff

	def same_size(self):
		return self.image1.size == self.image2.size

	def same_mode(self):
		return self.image1.mode == self.image2.mode
