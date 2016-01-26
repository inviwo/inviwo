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
import glob
import datetime
import json

from . import inviwoapp
from . import test
from .. util import *
from .. colorprint import *
from .. git import *
from . imagecompare import *
from . generatereport import *
from . database import *
from . reporttest import *


def findModuleTest(path):
	# assume path points to a folder of modules.
	# look in folder path/<module>/tests/regression/*
	tests = []
	for moduleDir in subDirs(path):
		regressionDir = toPath(path, moduleDir, "tests", "regression")
		for testDir in subDirs(regressionDir):
			tests.append(test.Test(
				name = testDir,
				module = moduleDir,
				path = toPath(regressionDir, testDir) 
				))
	return tests

class App:
	def __init__(self, appPath, outputPath, moduleTestPaths = [], settings = inviwoapp.RunSettings()):
		self.app = inviwoapp.InviwoApp(appPath, settings)
		self.output = outputPath
		tests = [findModuleTest(p) for p in moduleTestPaths]
		self.tests = list(itertools.chain(*tests))
		self.reports = {}
		self.git = Git(pyconfsearchpath = appPath)

	def runTest(self, test):
		report = {}
		report['date'] = datetime.datetime.now().isoformat()
		report = test.report(report)

		report = self.app.runTest(test, report, self.output)
		report = self.compareImages(test, report)
		testsuite = ReportTestSuite()
		report = testsuite.checkReport(report)

		return report


	def filterTests(self, testrange, testfilter):
		selected = range(len(self.tests))[testrange]
		selected = list(filter(lambda i: testfilter(self.tests[i]), selected))

		# don't run test from modules that we have not built
		selected = list(filter(lambda i: self.app.isModuleActive(self.tests[i].module), selected))
		return selected

	def runTests(self, testrange = slice(0,None), testfilter = lambda x: True):
		selected = self.filterTests(testrange, testfilter)

		for i,test in enumerate(self.tests):
			print_info("#"*80)
			if i in selected:
				print_pair("Running test {:3d} (Enabled: {:d}, Total: {:d})"
							.format(i, len(selected), len(self.tests)),
					test.toString())

				report = self.runTest(test)
				report['status'] = "new"
				report['git'] = self.git.info(report['path'])

				self.reports[test.toString()] = report
				for k,v in report.items():
					print_pair(k,str(v), width=15)

				if len(report["failures"])>0:
					print_error("{:>15} : {}".format("Result", "Failed " + ", ".join(report["failures"].keys())))
				else:
					print_good("{:>15} : {}".format("Result", "Success"))
				print()
			else:
				print_pair("Skipping test {:3d} (Enabled: {:d}, Total: {:d})"
							.format(i, len(selected), len(self.tests)),
					test.toString())

	def compareImages(self, test, report):
		refimgs = test.getImages()
		refs = set(refimgs)

		outputdir = report['outputdir']
		imgs = glob.glob(outputdir +"/imgtest/*.png")
		imgs = set([os.path.relpath(x, toPath(outputdir, 'imgtest')) for x in imgs])

		report["refs"] = list(refimgs)
		report["imgs"] = list(imgs)
		report['missing_refs'] = list(imgs - refs)
		report['missing_imgs'] = list(refs - imgs)

		imgtests = []
		for img in imgs:
			if img in refs:
				comp = ImageCompare(testImage = toPath(outputdir, "imgtest", img), 
					                refImage = toPath(test.path, img))

				diffpath = mkdir(toPath(outputdir, "imgdiff"))
				maskpath = mkdir(toPath(outputdir, "imgmask"))
				comp.saveDifferenceImage(toPath(diffpath, img), toPath(maskpath, img))
				refpath = mkdir(toPath(outputdir, "imgref"))
				comp.saveReferenceImage(toPath(refpath, img))

				diff = comp.difference()
				imgtest = {
					'image' : img,
					'difference' : diff,
				}
				imgtests.append(imgtest)

		report['image_tests'] = imgtests

		return report

	def printTestList(self, testrange = slice(0,None), testfilter = lambda x: True, printfun = print):
		printfun("List of regression tests")
		printfun("-"*80)

		selected = self.filterTests(testrange, testfilter)
		for i, test in enumerate(self.tests):
			active = "Enabled" if i in selected and testfilter(test) else "Disabled"
			printfun("{:3d} {:8s} {}".format(i, active, test))

	def saveJson(self, file):
		with open(file, 'w') as f:
			json.dump(self.reports, f, indent=4, separators=(',', ': '))

	def loadJson(self, file):
		with open(file, 'r') as f:
			self.reports = json.load(f)
		for name, report in self.reports.items():
			report['status'] = "old"

	def saveHtml(self, file, dbfile):
		dirname = os.path.dirname(file)
		html = HtmlReport(dirname, self.reports, dbfile)
		html.saveHtml(file)
		html.saveHtml(toPath(dirname, 
	    	"report-"+datetime.datetime.now().strftime('%Y-%m-%dT%H_%M_%S')+".html"))
    		
	def success(self):
		for name, report in self.reports.items():
			if len(report['failures']) != 0:
				if safeget(report, "config", "enabled", failure = True):
					return False
		return True

	def updateDatabase(self, file):
		db = Database(file)
		for name, report in self.reports.items():
			if report['status'] == "new":
				dbtest = db.getOrAddTest(report["module"], report["name"])
				dbtime = db.getOrAddQuantity("time", "s")
				dbcount = db.getOrAddQuantity("count", "")
				dbfrac = db.getOrAddQuantity("fraction", "%")
			
				db_elapsed_time = db.getOrAddSeries(dbtest, dbtime, "elapsed_time")
				db_test_failures = db.getOrAddSeries(dbtest, dbcount, "number_of_test_failures")

				db.addMeasurement(db_elapsed_time, report["elapsed_time"])
				db.addMeasurement(db_test_failures, len(report["failures"]))

				for img in report["image_tests"]:
					db_img_test = db.getOrAddSeries(dbtest, dbfrac, "image_test_diff." + img["image"])
					db.addMeasurement(db_img_test, img["difference"])


























