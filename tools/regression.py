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
import argparse

from ivwpy.util import *

# requirements
# python3
# yattag, http://www.yattag.org for html report generation
# PIL for image comparison
# sqlalchemy for database connection

missing_modules = {}

try:
	import yattag
except ImportError:
	missing_modules['yattag'] = "needed for html generation"

try:
	import PIL
except ImportError:
	missing_modules['PIL'] = "needed for image comparison"

try:
	import sqlalchemy
except ImportError:
	missing_modules['sqlalchemy'] = "needed for database connection"

if len(missing_modules)>0: 
	print_error("Error: Missing python modules:")
	for k,v in missing_modules.items():
		print_error("    {:20s} {}".format(k,v))	
	print_info("    To install run: 'pip3 install {}'".format(" ".join(missing_modules.keys())))
	exit()

import ivwpy.regression.app
import ivwpy.regression.error

# Ipython auto reaload
# %load_ext autoreload
# %autoreload 2

def makeCmdParser():
	parser = argparse.ArgumentParser(
		description="Run regression tests",
		formatter_class=argparse.ArgumentDefaultsHelpFormatter
	)
	parser.add_argument('-i', '--inviwo', type=str, required=True, action="store", dest="inviwo",
						help='Paths to inviwo executable')
	parser.add_argument('-o', '--output', type=str, required=True, action="store", dest="output",
						help='Path to output')
	parser.add_argument('-r', '--repos', type=str, nargs='*', action="store", dest="repos",
						help='Paths to inviwo repos')
	parser.add_argument("-m", "--modules", type=str, nargs='*', action="store", dest="modules", default=[],
						help="Paths to folders with modules")
	parser.add_argument("-t", "--tests", type=str, nargs='*', action="store", dest="tests", default=[],
						help="Paths to folders with tests")
	parser.add_argument("-s", "--slice", type=str, nargs='?', action="store", dest="slice", default = "", 
						help="Specifiy a specific slice of tests to run")
	parser.add_argument("--include", type=str, nargs='?', action="store", dest="include", default = "",
						help="Include filter")
	parser.add_argument("--exclude", type=str, nargs='?', action="store", dest="exclude", default = "", 
						help="Exclude filter")
	parser.add_argument("-l", "--list", action="store_true", dest="list", 
						help="List all tests")

	return parser.parse_args()


def searchRepoPaths(paths):
	modulePaths = []
	repoPaths = []
	for path in paths:
		if os.path.isdir(toPath([path, "modules"])):
			modulePaths.append(toPath([path, "modules"]))
		if os.path.isdir(toPath([path, "test", "regression"])):
			repoPaths.append(toPath([path, "test", "regression"]))

	return modulePaths, repoPaths

def makeFilter(inc, exc):
	if inc != "":
		def incfilter(test):
			return inc in test.toString()
	else:
		def incfilter(test):
			return True

	if exc != "":
		def excfilter(test):
			return exc in test.toString()
	else:
		def excfilter(test):
			return False

	def filter(test):
		return incfilter(test) and not excfilter(test)

	return filter

if __name__ == '__main__':

	args = makeCmdParser();

	modulePaths, repoPaths = searchRepoPaths(args.repos)
	modulePaths += args.modules
	repoPaths += args.tests

	modulePaths = map(os.path.abspath, modulePaths)
	repoPaths = map(os.path.abspath, repoPaths)
	output = os.path.abspath(args.output)
	inviwopath = os.path.abspath(args.inviwo)

	#base = "/Users/petst/Work/Projects/Inviwo-Developent/Private"
	#ivwapp = base + "/builds/cmake-test/bin/Debug/inviwo.app/Contents/MacOS/inviwo"

	#base = "C:/Users/petst55/Work/Inviwo"
	#ivwapp = base + "/Builds/build-git-VS2015-64bit-Qt5.5.0/bin/Debug/inviwo-cli.exe"

	#modulepaths = [base + "/inviwo-dev/modules", base + "/inviwo-research/modules"]
	#repopaths = [] #[base + "/inviwo-research/data/workspaces/tests/"]

	#out = base + "/regress"

	app = ivwpy.regression.app.App(inviwopath, output, modulePaths, repoPaths, 
		settings=ivwpy.regression.inviwoapp.RunSettings(timeout=60))

	testfilter = makeFilter(args.include, args.exclude)
	testrange = makeSlice(args.slice)

	if args.list:
		app.printTestList(testrange = testrange, testfilter = testfilter)
		exit()

	try: 
		app.runTests(testrange = testrange, testfilter = testfilter)
		app.updateDatabase(output + "/report.sqlite")
		app.saveJson(output+"/report.json")
		app.saveHtml(output+"/report.html")
	except ivwpy.regression.error.MissingInivioAppError as err:
		print_error(err.error)
		print_info("Check that option '-i' is correct")






