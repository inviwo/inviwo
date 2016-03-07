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

import time
import json
import inviwo

def saveCanvas(canvas, name = None):
	if name == None: name = canvas
	inviwo.snapshot( inviwo.getOutputPath() + "/imgtest/"+name+".png", canvas)

class Measurements:
	def __init__(self):
		self.m = []

	def add(self, name, quantity, unit, value):
		self.m.append({"name": name, 'quantity' : quantity, "unit": unit, 'value' : value})

	def addCount(self, name, value):
		self.add(name, "count", "", value)

	def addTime(self, name, value):
		self.add(name, "time", "s", value)

	def addFrequency(self, name, value):
		self.add(name, "frequency", "Hz", value)

	def addFraction(self, name, value):
		self.add(name, "fraction", "%", value)

	def save(self):
		with open(inviwo.getOutputPath() + "/stats.json", 'w') as f:
 			json.dump(self.m, f, indent=4, separators=(',', ': '))

