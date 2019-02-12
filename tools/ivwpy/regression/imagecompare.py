#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2019 Inviwo Foundation
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
import PIL.ImageMath as ImageMath
import PIL.ImageStat as ImageStat
import PIL.ImageChops as ImageChops

class ImageCompare:
    def __init__(self, testImage, refImage, allowDifferentImageMode = False , enhance = 10):

        self.testImage = Image.open(testImage)
        self.refImage = Image.open(refImage)

        self.maskImage = None
        self.diffImage = None
        self.difference = 100
        self.numberOfDifferentPixels = None
        self.maxDifference = None


        if self.testImage.size != self.refImage.size:
            return

        if self.testImage.mode != self.refImage.mode:
            if not allowDifferentImageMode:
                return;
            self.refImage = self.refImage.convert(self.testImage.mode)


        # ImageChops.difference does not work for signed integers
        if self.testImage.mode == 'I':
            self.testImage = self.testImage.convert('L')
            self.refImage = self.refImage.convert('L')


        numPixels = self.testImage.size[0] * self.testImage.size[1]

        self.diffImage = ImageChops.difference(self.testImage, self.refImage)

        channels = len(self.testImage.getbands());
        if channels == 1:
            normImage = self.diffImage
        elif channels == 2:
            (a,b) = self.diffImage.split()
            normImage = ImageMath.eval("convert((a+b), 'L')" , a=a, b=b)
        elif channels == 3:
            (a,b,c) = self.diffImage.split()
            normImage = ImageMath.eval("convert((a+b+c), 'L')" , a=a, b=b, c=c)
        elif channels == 4:
            (a,b,c,d) = self.diffImage.split()
            normImage = ImageMath.eval("convert((a+b+c+d), 'L')" , a=a, b=b, c=c, d=d)
        
        self.maskImage = normImage.point(lambda p: 0 if p > 0 else 255, 'L')
        
        stats = ImageStat.Stat(normImage)
        self.maxDifference = stats.extrema[0][1] / (channels*255)
        self.difference = (sum(stats.sum)/(255*channels)) * 100.0 / numPixels

        if enhance != 1:
            self.diffImage = self.diffImage.point(lambda i : i * (enhance))
        self.diffImage = ImageChops.invert(self.diffImage)

        self.numberOfDifferentPixels = int(numPixels - ImageStat.Stat(self.maskImage).sum[0] / 255)

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
