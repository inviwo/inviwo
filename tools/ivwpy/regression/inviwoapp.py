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

import io
import os
import time
import subprocess

from . error import *
from .. util import *

class RunSettings:
	def __init__(self, timeout = 15, activeModules = None):
		self.timeout = timeout
		self.activeModules = None if activeModules is None else [x.casefold() for x in activeModules]


class InviwoApp:
	def __init__(self, appPath, settings = RunSettings()):
		self.program = appPath
		self.settings = settings

	def isModuleActive(self, name):
		if self.settings.activeModules is None: 
			return True
		else:
			return name.casefold() in self.settings.activeModules

	def runTest(self, test, report, output):
		report['outputdir'] = mkdir(test.makeOutputDir(base=output), report['date'].replace(":","_"))

		mkdir(report['outputdir'], "imgtest")

		for workspace in test.getWorkspaces():
			starttime = time.time()
		
			command = [self.program, 
						"-q",
						"-o", report['outputdir'], 
						"-g", "screenshot.png",
						"-s", "imgtest/UPN", 
						"-l", "log.txt",
						"-w", workspace]
			if report["script"] != "": command += ["-p", report["script"]]

			report['command'] = " ".join(command)
			report['timeout'] = False

			try:
				with subprocess.Popen(
					command,
					cwd = os.path.dirname(self.program),
					stdout=subprocess.PIPE, 
					stderr=subprocess.PIPE,
					universal_newlines = True
					) as process:

					try:
						report["output"], report["errors"] = process.communicate(
							timeout=self.settings.timeout)
					except subprocess.TimeoutExpired as e:
						report['timeout'] = True
						process.kill()
						report["output"], report["errors"] = process.communicate()

			except FileNotFoundError:
				raise MissingInivioAppError("Could not find inviwo app at: {}".format(self.program))

			report['log'] = "log.txt"
			report['screenshot'] = "screenshot.png"
			report['returncode'] = process.returncode
			report['elapsed_time'] = time.time() - starttime

			return report



		
