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
from . database import *

def elapsed_time_stats(database):
	'''
	database should be a sql regression database
	'''
	timedelta = 3600;
	total = {}

	for module in database.getModules():
		measurements = {}

		for test in module.tests:
			series = database.getSeries(module.name, test.name, "elapsed_time")

			data = {}
			for m in series.measurements:
				time = round(m.created.timestamp()/timedelta)
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

	return [[datetime.datetime.fromtimestamp(k*timedelta), y] for k,y in sorted(totdata.items(), key = lambda a: a[0])]


def result_time_stats(database):
	timedelta = 3600;
	result = {} 
	for module in database.getModules():
		for test in module.tests:
			testfail = {}
			testpass = {}
			for testrun in test.testruns:
				time = round(testrun.created.timestamp()/timedelta)
				if len(testrun.testfailures) == 0:
					testpass[time] = 1
					testfail.pop(time, None)
				else:
					testfail[time] = 1
					testpass.pop(time, None)

			for time, val in testpass.items():
				res = result.setdefault(time, [0,0])
				res[0] += val
				result[time] = res

			for time, val in testfail.items():
				res = result.setdefault(time, [0,0])
				res[1] += val
				result[time] = res

	#print(", ".join(["{:}/{:}".format(y[1],y[0]+y[1]) for k,y in sorted(result.items(), key = lambda a: a[0])]))
	return [[datetime.datetime.fromtimestamp(k*timedelta), y] for k,y in sorted(result.items(), key = lambda a: a[0])]

