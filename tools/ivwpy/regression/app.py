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
import shutil
import filecmp

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
	def __init__(self, 
		         appPath, 
				 moduleTestPaths, 
				 outputDir,
				 jsonFile = "report",
				 htmlFile = "report",
				 sqlFile = "report",
				 runSettings = inviwoapp.RunSettings(),
				 testSettings = ReportTestSettings()):

		self.app = inviwoapp.InviwoApp(appPath, runSettings)
		self.output = outputDir
		self.jsonFile = jsonFile
		self.htmlFile = htmlFile
		self.sqlFile = sqlFile
		self.testSettings = testSettings

		tests = [findModuleTest(p) for p in moduleTestPaths]
		self.tests = list(itertools.chain(*tests))
		self.reports = {}
		self.git = Git(pyconfsearchpath = appPath)
		if not self.git.foundGit():
			print_error("Git not found")
			exit(1)
		
		self.db = Database(self.output+ "/" + self.sqlFile + ".sqlite")
		self.loadJson()


	def runTest(self, test):
		report = {}
		report['date'] = datetime.datetime.now().isoformat()
		report = test.report(report)

		report = self.app.runTest(test, report, self.output)
		report = self.compareImages(test, report)

		testsuite = ReportTestSuite(self.testSettings)
		report = testsuite.checkReport(report)

		report['status'] = "new"
		report['git'] = self.git.info(report['path'])

		self.updateDatabase(report)
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
		report['image_tests'] = []

		olddirs = list(reversed(sorted(glob.glob(outputdir+"/../*"))))
		if len(olddirs) > 1:
			lastoutdir = olddirs[1]
		else:
			lastoutdir = None

		testpath = mkdir(toPath(outputdir, "imgtest"))
		refpath = mkdir(toPath(outputdir, "imgref"))
		diffpath = mkdir(toPath(outputdir, "imgdiff"))
		maskpath = mkdir(toPath(outputdir, "imgmask"))

		for img in imgs & refs:
			comp = ImageCompare(testImage = toPath(testpath, img), 
				                refImage = toPath(test.path, img))

			comp.saveReferenceImage(toPath(refpath, img))
			comp.saveDifferenceImage(toPath(diffpath, img))
			comp.saveMaskImage(toPath(maskpath, img))
			
			# do some hardlinks to save disk space...
			if lastoutdir != None:
				self.linkImages(toPath(lastoutdir, "imgtest", img), toPath(testpath, img))
				self.linkImages(toPath(lastoutdir, "imgref", img), toPath(refpath, img))
				self.linkImages(toPath(lastoutdir, "imgdiff", img), toPath(diffpath, img))
				self.linkImages(toPath(lastoutdir, "imgmask", img), toPath(maskpath, img))

			imgtest = {
				'image' : img,
				'difference' : comp.getDifference(),
				'max_difference' : comp.getMaxDifference(),
				'different_pixels' : comp.getNumberOfDifferentPixels(),
				'test_size' : comp.getTestSize(),
				'ref_size' : comp.getRefSize(),
				'test_mode' : comp.getTestMode(),
				'ref_mode' : comp.getRefMode()
			}
			report['image_tests'].append(imgtest)

		return report

	def linkImages(self, oldimg, newimg):
		if os.path.exists(oldimg) and os.path.exists(newimg) and filecmp.cmp(oldimg, newimg):
			os.remove(newimg)
			os.link(oldimg, newimg)

	def printTestList(self, testrange = slice(0,None), testfilter = lambda x: True, printfun = print):
		printfun("List of regression tests")
		printfun("-"*80)

		selected = self.filterTests(testrange, testfilter)
		for i, test in enumerate(self.tests):
			active = "Enabled" if i in selected and testfilter(test) else "Disabled"
			printfun("{:3d} {:8s} {}".format(i, active, test))

	def saveJson(self):
		with open(self.output+ "/" + self.jsonFile + ".json", 'w') as f:
			json.dump(self.reports, f, indent=4, separators=(',', ': '))

	def loadJson(self):
		if os.path.exists(self.output+ "/" + self.jsonFile + ".json"):
			with open(self.output+ "/" + self.jsonFile + ".json", 'r') as f:
				self.reports = json.load(f)
			for name, report in self.reports.items():
				report['status'] = "old"

	def saveHtml(self):
		html = HtmlReport(basedir = self.output, reports = self.reports, database = self.db)
		filepath = html.saveHtml(self.htmlFile)
		shutil.copyfile(filepath, toPath(self.output, 
	    	self.htmlFile + "-" + datetime.datetime.now().strftime('%Y-%m-%dT%H_%M_%S')+".html"))
    		
	def success(self):
		for name, report in self.reports.items():
			if len(report['failures']) != 0:
				if safeget(report, "config", "enabled", failure = True):
					return False
		return True

	def updateDatabase(self, report):
		dbtest = self.db.getOrAddTest(report["module"], report["name"])
		dbtime = self.db.getOrAddQuantity("time", "s")
		dbcount = self.db.getOrAddQuantity("count", "")
		dbfrac = self.db.getOrAddQuantity("fraction", "%")
		
		db_elapsed_time = self.db.getOrAddSeries(dbtest, dbtime, "elapsed_time")
		db_test_failures = self.db.getOrAddSeries(dbtest, dbcount, "number_of_test_failures")

		git = report["git"]
		dbcommit = self.db.getOrAddCommit(hash    = git["hash"],
				                     	  date    = stringToDate(git['date']), 
				                    	  author  = git['author'], 
				                     	  message = git['message'], 
				                          server  = git['server']) 

		dbtestrun = self.db.addTestRun(test = dbtest, commit = dbcommit)

		self.db.addMeasurement(series  = db_elapsed_time, 
							   testrun = dbtestrun, 
							   value   = report["elapsed_time"])
		self.db.addMeasurement(series = db_test_failures, 
			                   testrun = dbtestrun, 
			                   value = len(report["failures"]))


		for key, messages in report['failures'].items():
			for message in messages:
				self.db.addTestFailure(testrun = dbtestrun, key = key, message = message)

		for img in report["image_tests"]:
			db_img_test = self.db.getOrAddSeries(dbtest, dbfrac, "image_test_diff." + img["image"])
			self.db.addMeasurement(series = db_img_test, 
				                   testrun = dbtestrun, 
				                   value = img["difference"])

		if os.path.exists(report['outputdir'] + "/stats.json"):
			stats = []
			with open(report['outputdir'] + "/stats.json", 'r') as f:
				stats = json.load(f)

			for stat in stats:
				unit = self.db.getOrAddQuantity(stat["quantity"], stat["unit"])
				series = self.db.getOrAddSeries(dbtest, unit, stat["name"])
				self.db.addMeasurement(series = series, testrun = dbtestrun, value = stat["value"])




























