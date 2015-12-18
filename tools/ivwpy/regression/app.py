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
import itertools

from . import inviwoapp
from . import test
from .. import util

class App:
	def __init__(self, appPath, moduleTestPaths = [], repoTestPaths = []):
		self.app = inviwoapp.InviwoApp(appPath)
		tests = [self.findModuleTest(p) for p in moduleTestPaths] + [self.findRepoTest(p) for p in repoTestPaths]
		self.tests = list(itertools.chain(*tests))

	def findModuleTest(self, path):
		# assume path points to a folder of modules.
		# look in folder path/<module>/tests/regression/*
		tests = []

		for moduleDir in util.subDirs(path):
			regressionDir = util.toPath([path, moduleDir, "tests", "regression"])
			for testDir in util.subDirs(regressionDir):
				tests.append(test.Test(
					kind = "module", 
					name = testDir,
					module = moduleDir,
					path = util.toPath([regressionDir, testDir]) 
					))
		return tests

	def findRepoTest(self, path):
		# assume path points to a repo.
		# look for tests in path/tests/regression
		tests = []
		regressionDir = util.toPath([path, "tests", "regression"])
		for testDir in util.subDirs(regressionDir):
			tests.append(test.Test(
						kind = "repo", 
						name = testDir,
						repo = path.split("/")[-1],
						path = util.toPath([regressionDir, testDir]) 
						))
		return tests

	def runTests(self):
		for test in self.tests:
			self.app.runTest(test)


