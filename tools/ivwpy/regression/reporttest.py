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

from .. util import *

class ReportTestSettings:
	def __init__(self, imageDifferenceTolerance = 0.0):
		self.imageDifferenceTolerance = imageDifferenceTolerance

class ReportTest:
	def __init__(self, key, testfun, message):
		self.key = key
		self.testfun = testfun
		self.message = message

	def test(self, report):
		return self.testfun(report[self.key])

	def failures(self):
		return {self.key : [self.message]}

class ReportImageTest(ReportTest):
	def __init__(self, key, differenceTolerance = 0.0):
		self.key = key
		self.message = {}
		self.differenceTolerance = differenceTolerance

	def test(self, report):
		imgs = report[self.key]
		for img in imgs:
			tol = safeget(report, "config", "image_test", "differenceTolerance", img["image"], failure = self.differenceTolerance)

			if img['test_mode'] != img['ref_mode']:
				self.message[img['image']] = \
					("Image {image} has different modes, " +
					"Test: {test_mode} vs Reference:{ref_mode}").format(**img)
			elif img['test_size'] != img['ref_size']:
				self.message[img['image']] = \
					("Image {image} has different sizes, " +
					"Test: {test_size} vs Reference:{ref_size}").format(**img)
			elif img["difference"] > tol:
				self.message[img['image']] = \
					("Image {image} has difference greater then the allowd tolerance ({difference}% &gt; {tol})  " +
					"difference, {different_pixels} different pixels, " +
					"largest difference {max_difference}").format(tol=tol, **img)

		return len(self.message) == 0

	def failures(self):
		return {self.key : self.message}

class ReportLogTest(ReportTest):
	def __init__(self):
		self.key = 'log'
		self.message = []

	def test(self, report):
		with open(toPath(report['outputdir'], report['log']), 'r') as f:
			lines = f.readlines()
			for line in lines:
				if "Error:" in line:
					self.message.append(line)
			
		return len(self.message) == 0

	def failures(self):
		return {"log" : self.message}


class ReportTestSuite:
	def __init__(self, settings = ReportTestSettings()):
		self.settings = settings
		self.tests = [
			ReportTest('returncode', lambda x : x == 0, "Non zero retuncode"),
			ReportTest('timeout', lambda x : x == False, "Inviwo ran out of time"),
			ReportTest('missing_refs', lambda x : len(x) == 0, "Missing refecence image"),
			ReportTest('missing_imgs', lambda x : len(x) == 0, "Missing test image"),
			ReportImageTest('image_tests', settings.imageDifferenceTolerance),
			ReportLogTest()
		]

	def checkReport(self, report):
		failures = {}
		successes = []
		for t in self.tests:
			if not t.test(report):
				failures.update(t.failures())
			else:
				successes.append(t.key)
		report['failures'] = failures
		report['successes'] = successes
		return report

