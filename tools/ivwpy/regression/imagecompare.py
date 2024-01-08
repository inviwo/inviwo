# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2024 Inviwo Foundation
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
# ********************************************************************************

import PIL.Image as Image

import numpy as np
import shutil

class ImageCompare:
    def __init__(self, testImage, refImage, allowDifferentImageMode=False, 
        logscaleDifferenceImage=False, invertDifferenceImage=False, diffScale = 10.0):

        self.image1 = { 'filename' : None, 'size' : None, 'mode' : None }
        self.image2 = { 'filename' : None, 'size' : None, 'mode' : None }

        self.diffPercent = 100
        self.diffPixelCount = 0
        self.maxDifferences = None
        self.diffImage = None
        self.maskImage = None

        self.logscaleImage = logscaleDifferenceImage
        self.invertImage = invertDifferenceImage
        self.diffScale = diffScale

        self.diff(refImage, testImage)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.diffImage is not None:
            self.diffImage.close()
        if self.maskImage is not None:
            self.maskImage.close()

    def diff(self, image1, image2):
        self.image1['filename'] = image1
        self.image2['filename'] = image2

        with Image.open(image1) as img1, Image.open(image2) as img2:

            if img1.mode == 'P':
                img1 = img1.convert(img1.palette.mode)
            if img2.mode == 'P':
                img2 = img1.convert(img2.palette.mode)

            self.image1['size'] = img1.size
            self.image1['mode'] = img1.mode
            self.image2['size'] = img2.size
            self.image2['mode'] = img2.mode

            if img1.size != img2.size:
                return
            if img1.mode != img2.mode:
                return

            # apply heuristics to determine bit depth since PIL does not provide it
            #
            # see https://pillow.readthedocs.io/en/stable/handbook/concepts.html
            if img1.mode.startswith('I'):
                maxval = 65535
            elif img1.mode == 'F':
                maxval = 1
            else:
                maxval = 255

            img1np = np.array(img1)
            img2np = np.array(img2)

            diffnp = np.abs(img1np.astype(np.int32) - img2np.astype(np.int32))

            # normalized differences per channel
            if len(diffnp.shape) > 2:
                self.maxDifferences = tuple(np.max(diffnp, axis=(0, 1)) / maxval)
            else:
                self.maxDifferences = (np.max(diffnp, axis=(0, 1)) / maxval,)

            # combine all channels into one
            if len(diffnp.shape) > 2:
                diffnp = diffnp.max(2)

            # calculate stats
            numpixels = np.prod(img1np.shape[0:2])
            self.diffPixelCount = np.count_nonzero(diffnp)
            self.diffPercent = self.diffPixelCount / numpixels * 100.0

            if self.logscaleImage:
                diffnp = np.log(diffnp + 1)/np.log(maxval + 1) * maxval
            if self.invertImage:
                diffnp = maxval - diffnp

            if maxval != 255:
                diffnp = diffnp * 255 / maxval

            # scale diff image and ensure 8bit
            diffnp = np.clip(diffnp * self.diffScale, 0.0, 255.0).astype(np.int8)

            self.diffImage = Image.fromarray(diffnp, mode='L')
            self.maskImage = Image.fromarray((diffnp == 0))

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
        if self.image1['filename'] is not None:
            # copy original file
            shutil.copyfile(self.image1['filename'], file)

    def isSameSize(self):
        return self.image1['size'] == self.image2['size']

    def isSameMode(self):
        return self.image1['mode'] == self.image2['mode']

    def getDifferencePercent(self):
        """
        return the number of different pixels as percentage
        :return: getNumberOfDifferentPixels() normalized with respect to number of pixels * 100
        """
        return self.diffPercent

    def getDifferencePixelCount(self):
        """
        return the number of different pixels in the image (combined over all channels including alpha)
        """
        return self.diffPixelCount

    def getMaxDifferences(self):
        """
        returns a tuple of the maximum difference per channel
        :return: tuple with per-channel maxima normalized with respect to image bit depth
        """
        return self.maxDifferences

    def getRefSize(self):
        return self.image1['size']

    def getRefMode(self):
        return self.image1['mode']

    def getTestSize(self):
        return self.image2['size']

    def getTestMode(self):
        return self.image2['mode']
