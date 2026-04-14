# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2026 Inviwo Foundation
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

from numpy import angle

import inviwopy
import math
from inviwopy.glm import dmat3, dvec3


class Camera:
    """
    Example:
    with ivw.camera.Camera(app.network.EntryExitPoints.camera) as c:
        for step in c.rotate(2.0*math.pi/steps, steps, [0,0,1]):
            print(step)
            inviwoqt.update()
    """

    def __init__(self, cameraProperty, lookFrom=None, lookTo=None, lookUp=None):
        self.cam = cameraProperty
        self.oldLookFrom: dvec3 = cameraProperty.lookFrom
        self.oldLookTo: dvec3 = cameraProperty.lookTo
        self.oldLookUp: dvec3 = cameraProperty.lookUp

        self.lookFrom: dvec3 = lookFrom if lookFrom is not None else self.oldLookFrom
        self.lookTo: dvec3 = lookTo if lookTo is not None else self.oldLookTo
        self.lookUp: dvec3 = lookUp if lookUp is not None else self.oldLookUp

    def __enter__(self):
        self.set()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.restore()

    def set(self):
        self.cam.setLook(self.lookFrom, self.lookTo, self.lookUp)

    def restore(self):
        self.cam.setLook(self.oldLookFrom, self.oldLookTo, self.oldLookUp)

    @staticmethod
    def rotation_matrix(axis: dvec3, angle: float) -> dmat3:
        a = inviwopy.glm.normalize(axis)
        v = a * math.sin(0.5 * angle)
        w = math.cos(0.5 * angle)

        m = dmat3()
        m[0][0] = 1 - (2) * (v[1] * v[1] + v[2] * v[2])
        m[1][0] = 2 * (v[0] * v[1] + v[2] * w)
        m[2][0] = 2 * (v[2] * v[0] - v[1] * w)

        m[0][1] = 2 * (v[0] * v[1] - v[2] * w)
        m[1][1] = 1 - (2) * (v[2] * v[2] + v[0] * v[0])
        m[2][1] = 2 * (v[1] * v[2] + v[0] * w)

        m[0][2] = 2 * (v[2] * v[0] + v[1] * w)
        m[1][2] = 2 * (v[1] * v[2] - v[0] * w)
        m[2][2] = 1 - (2) * (v[1] * v[1] + v[0] * v[0])

        return m

    def rotate(self, delta: float = math.pi / 30, steps: int = 60, axis: dvec3 = None):
        if axis is None:
            axis = self.lookUp

        vec = self.lookFrom - self.lookTo
        mat = self.rotation_matrix(axis, delta)

        for i in range(1, steps + 1):
            vec = mat * vec
            up = mat * self.lookUp
            self.lookFrom = vec + self.lookTo
            self.lookUp = up
            self.set()
            yield i
