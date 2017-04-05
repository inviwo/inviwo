#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2017 Inviwo Foundation
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

import inviwopy

from inviwopy.glm import ivec2,vec3,mat3

import math
#import numpy as np

def rotation_matrix(axis, angle):
    a = axis.normalize()
    v = a * math.sin(0.5 * angle)
    w = math.cos(0.5 * angle)

    #m = np.zeros([3,3])
    m = mat3()

    a = 1 - (2) * (v[1] * v[1] + v[2] * v[2]);
    b = 2 * (v[0] * v[1] + v[2] * w);
    c = 2 * (v[2] * v[0] - v[1] * w);

    m[0] = vec3(a,b,c);

    a = 2 * (v[0] * v[1] - v[2] * w);
    b = 1 - (2) * (v[2] * v[2] + v[0] * v[0]);
    c = 2 * (v[1] * v[2] + v[0] * w);

    m[1] = vec3(a,b,c);

    a = 2 * (v[2] * v[0] + v[1] * w);
    b = 2 * (v[1] * v[2] - v[0] * w);
    c = 1 - (2) * (v[1] * v[1] + v[0] * v[0]);

    m[2] = vec3(a,b,c);


    return m


class Camera:
    """
    Example:
    with ivw.camera.Camera("EntryExitPoints.camera") as c:
        for step in c.rotate(2.0*math.pi/steps, steps, [0,0,1]): 
            print(step)
            inviwoqt.update()
    """
    def __init__(self, cameraProperty, lookfrom = None, lookto = None, lookup = None):
        self.cam = cameraProperty
        self.oldlookfrom = cameraProperty.lookFrom
        self.oldlookto = cameraProperty.lookTo
        self.oldlookup = cameraProperty.lookUp

        self.lookfrom = lookfrom if lookfrom != None else self.oldlookfrom
        self.lookto = lookto if lookto != None else self.oldlookto
        self.lookup = lookup if lookup != None else self.oldlookup

    def __enter__(self):
        self.set()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.restore()

    def set(self):
        self.cam.lookFrom = self.lookfrom;
        self.cam.lookTo = self.lookto;
        self.cam.lookUp = self.lookup;
        self.cam.invalidate()

    def restore(self):
        self.cam.lookFrom = self.oldlookfrom;
        self.cam.lookTo = self.oldlookto;
        self.cam.lookUp = self.oldlookup;
        self.cam.invalidate()

    def rotate(self, delta = math.pi/30, steps = 60, axis = None):
        if axis == None: axis = self.lookup
        
        vec = self.lookfrom - self.lookto

        mat = rotation_matrix(axis, delta)
        
        print(vec)
        print(mat)
        print(self.lookup)
        print(self.lookfrom)
        print(self.lookto)
        print("")

        for i in range(1, steps+1):
            vec = mat * vec
            up = mat * self.lookup
            self.lookfrom = vec + self.lookto
            self.lookup = up
            self.set()
            yield i