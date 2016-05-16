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

import json

from .. util import *
from . database import *

def elapsed_time_stats(database):
	'''
	database should be a sql regression database
	'''
	total = {}
	for module in database.getModules():
		measurements = {}

		for test in module.tests:
			series = database.getSeries(module.name, test.name, "elapsed_time")
			if series == None: continue

			data = {}
			for m in series.measurements:
				time = m.testrun.run.created.timestamp()
				if time in data.keys():
					data[time]["count"] += 1
					data[time]["time"] += m.value
				else:
					data[time] = {"count" : 1, "time" : m.value}
			
			tot = 0
			n = 0
			for time, item in data.items():
				norm = item["time"] / item["count"]
				data[time] = norm
				tot += norm
				n += 1

			for time, val in data.items():
				data[time] = val / (tot / n) 

			for time, val in data.items():
				if time in total.keys():
					total[time]["count"] += 1
					total[time]["time"] += val
				else:
					total[time] = {"count" : 1, "time" : val}

	totdata = {}
	tot = 0
	n = 0
	for time, item in total.items():
		norm = item["time"] / item["count"]
		tot += norm
		n += 1
		totdata[time] = norm 

	for time, val in totdata.items():
		totdata[time] = val / (tot / n) 

	#print(", ".join(["{:5.2f}".format(y) for k,y in sorted(totdata.items(), key = lambda a: a[0])]))

	return [[datetime.datetime.fromtimestamp(k), y] for k,y in sorted(totdata.items(), key = lambda a: a[0])]


def result_time_stats(database):
	'''
	database should be a sql regression database
	'''
	result = []
	for run in database.getRuns():
		testfail = 0
		testpass = 0
		testskip = 0
		for testrun in run.testruns:
			config = json.loads(testrun.config)
			enabled = safeget(config, "enabled", failure = True)

			if not enabled:
				testskip += 1
			elif len(testrun.failures) == 0:
				testpass += 1
			else:
				testfail += 1

		testskip += len(run.skipruns)

		result.append([datetime.datetime.fromtimestamp(run.created.timestamp()), [testpass, testfail, testskip]])

	return result

def get_plot_data(database):
	'''
	database should be a sql regression database
	'''
	result = {}
	for module in database.getModules():
		for test in module.tests:
			lines = {}
			for series in test.serieses:
				line = []
				for m in series.measurements:
					line.append([m.created.timestamp()*1000, m.value])
				lines[series.name] = {
						"data" : line,
						"unit" : series.quantity.unit
					}
			result[module.name + "/" + test.name] = lines
	return result
