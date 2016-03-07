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

import os
import re
import configparser
import subprocess

from . import util
from . import colorprint as cp
from . import ivwpaths

class ModuleRegistrationError(BaseException):
	def __init__(self, error):
		self.error = error

class ModuleRegistration:
	"""Represents a moudule registration file, i.e. basemodule.cpp"""
	def __init__(self, filepath):
		self.cppfile = filepath + ".cpp"
		self.lines = []
		with open(self.cppfile, "r") as f:
			for l in f:
				self.lines.append(l)

	def addProcessor(self, processor, header):
		mConstructor = re.compile(r"""(\s*\w+)Module::\1Module\(InviwoApplication\* \w+\)\s*:\s*InviwoModule\(\w*\s*,\s*"\w*"\)\s*{""")
		mProcessor = re.compile(r"""\s*registerProcessor<\w+>\(\);""")
		mInclude = re.compile(r"""#include\s* <[\w/.]*>""")

		lastInclude = None
		constructorStart = None
		lastProcessor = None

		for i,line in enumerate(self.lines):
			if mInclude.match(line): lastInclude = i
			elif mConstructor.match(line): constructorStart = i
			elif mProcessor.match(line): lastProcessor = i

		if mConstructor ==  None: raise ModuleRegistrationError("Error: Cound not find construcor")
		if lastInclude ==  None: raise ModuleRegistrationError("Error: Cound not find includes")
		if lastInclude >= constructorStart: raise ModuleRegistrationError("Error: we do not understand the file structure")
		
		self.lines.insert(lastInclude+1, "#include " + header + "\n")
		ppos = lastProcessor+2 if lastProcessor != None else constructorStart + 2
		self.lines.insert(ppos, "    registerProcessor<" + processor + ">();\n")

	def write(self, file = ""):
		with open(file +".cpp" if file != "" else self.cppfile, "w") as f:
			print("... Updating object registration: " + f.name)
			for l in self.lines:
				f.write(l)