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

from inviwopy.properties import Property

class Animation:
    """
    Example:
    import inviwopy
    from inviwopy import qt
    import ivw.animation

    app = inviwopy.app
    network = app.network

    with ivw.animation.Animation(network.VolumeRaycaster.raycaster.samplingRate,1,10,0.5) as ani:
        for i,v in ani:
            print([i,v])
            qt.update()
    """
    def __init__(self, prop : Property, start = None, stop = None, inc = None):
        self.prop = prop
        self.startVal = start if start != None else prop.minValue
        self.stopVal = stop if stop != None else prop.maxValue
        self.inc = inc if inc != None else 1
        self.initalVal = prop.value
        self.i = 0

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.restore()

    def __iter__(self):
        return self

    def __next__(self):
        self.i = self.i + 1
        if self.startVal + self.i * self.inc < self.stopVal:
            self.prop.value = self.startVal + self.i * self.inc
            return (self.i,self.prop.value)
        else:
            raise StopIteration

    def pos(self):
        return self.startVal + self.i * self.inc

    def start(self):
        self.i = 0
        self.prop.value = self.startVal

    def restore(self):
        self.i = 0
        self.prop.value = self.initalVal




