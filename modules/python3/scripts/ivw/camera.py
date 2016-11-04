#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2016 Inviwo Foundation
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

import inviwo 

import math
import numpy as np

def rotation_matrix(axis, angle):
    a = np.asarray(axis)
    a = a / np.linalg.norm(a)
    v = a * math.sin(0.5 * angle)
    w = math.cos(0.5 * angle)

    m = np.zeros([3,3])
    m[0][0] = 1 - (2) * (v[1] * v[1] + v[2] * v[2]);
    m[0][1] = 2 * (v[0] * v[1] + v[2] * w);
    m[0][2] = 2 * (v[2] * v[0] - v[1] * w);

    m[1][0] = 2 * (v[0] * v[1] - v[2] * w);
    m[1][1] = 1 - (2) * (v[2] * v[2] + v[0] * v[0]);
    m[1][2] = 2 * (v[1] * v[2] + v[0] * w);

    m[2][0] = 2 * (v[2] * v[0] + v[1] * w);
    m[2][1] = 2 * (v[1] * v[2] - v[0] * w);
    m[2][2] = 1 - (2) * (v[1] * v[1] + v[0] * v[0]);
    return m


class Camera:
    """
    Example:
    with ivw.camera.Camera("EntryExitPoints.camera") as c:
        for step in c.rotate(2.0*math.pi/steps, steps, [0,0,1]): 
            print(step)
            inviwoqt.update()
    """
    def __init__(self, id, lookfrom = None, lookto = None, lookup = None):
        self.id = id
        (self.oldlookfrom, self.oldlookto, self.oldlookup) = inviwo.getPropertyValue(self.id)
        self.lookfrom = lookfrom if lookfrom != None else self.oldlookfrom
        self.lookto = lookto if lookto != None else self.oldlookto
        self.lookup = lookup if lookup != None else self.oldlookup

    def __enter__(self):
        self.set()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.restore()

    def set(self):
        inviwo.setPropertyValue(self.id, (self.lookfrom, self.lookto, self.lookup))

    def restore(self):
        inviwo.setPropertyValue(self.id, (self.oldlookfrom, self.oldlookto, self.oldlookup))

    def rotate(self, delta = math.pi/30, steps = 60, axis = None):
        if axis == None: axis = self.lookup
        lookfrom = np.asarray(self.lookfrom)
        lookto = np.asarray(self.lookto)
        vec = lookfrom - lookto
        up = np.asarray(self.lookup)
        mat = rotation_matrix(axis, delta)
        for i in range(1, steps+1):
            vec = np.dot(mat, vec)
            up = np.dot(mat, up)
            self.lookfrom = vec + lookto
            self.lookup = up
            self.set()
            yield i