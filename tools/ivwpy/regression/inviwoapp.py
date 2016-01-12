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
import glob
import subprocess
import datetime

from . error import *
from . imagecompare import *
from .. util import *

def mkdir(path):
	if isinstance(path, (list, tuple)):
		path = "/".join(path)
	if not os.path.isdir(path):
		os.mkdir(path)

def makeOutputDir(base, test):
	if not os.path.isdir(base):
		raise RegressionError("Output dir does not exsist: " + dir)

	if test.kind == "module" :
		mkdir([base, test.module])
		mkdir([base, test.module, test.name])
		return "/".join([base, test.module, test.name])

	elif test.kind == "repo":
		mkdir([base, test.repo])
		mkdir([base, test.repo, test.name])
		return "/".join([base, test.repo, test.name])

	raise RegressionError("Invalid Test kind")

class InviwoApp:
	def __init__(self, appPath):
		self.program = appPath

	def runTest(self, test, output):
		print_info("#"*80)

		
				
		outputdir = makeOutputDir(output, test)

		for workspace in test.getWorkspaces():

			starttime = time.time()

			report = {}
			report['module'] = test.module
			report['name'] = test.name
			report['date'] = datetime.datetime.now().isoformat()

			command = [self.program, 
						"-q",
						"-o", outputdir, 
						"-g", "screenshot.png",
						"-s", "UPN", 
						"-l", "log.txt",
						"-w", workspace]
			report['command'] = " ".join(command)

			report['timeout'] = False

			process = subprocess.Popen(
				command,
				cwd = os.path.dirname(self.program),
				stdout=subprocess.PIPE, 
				stderr=subprocess.PIPE,
				universal_newlines = True
			)

			try:
				report["output"], report["errors"] = process.communicate(timeout=15)
			except subprocess.TimeoutExpired as e:
				report['timeout'] = True
				process.kill()
				report["output"], report["errors"] = process.communicate()
			
			report['log'] = outputdir + "log.txt"
			report['returncode'] = process.returncode
			report['elapsed_time'] = time.time() - starttime

			for k,v in report.items():
				print_pair(k,str(v))


			refimgs = test.getImages()
			imgs = glob.glob(outputdir +"/*.png")

			print(refimgs)
			print(imgs)


			print()



		
