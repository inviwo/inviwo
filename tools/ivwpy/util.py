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
import datetime
import math

def subDirs(path):
	if os.path.isdir(path):
		return next(os.walk(path))[1]
	else:
		return []

def toPath(*list):
	return "/".join(list)

def addPostfix(file, postfix):
	parts = file.split(os.path.extsep)
	parts[0]+= postfix
	return os.path.extsep.join(parts)

def in_directory(file, directory):
    #make both absolute    
    directory = os.path.join(os.path.realpath(directory), '')
    file = os.path.realpath(file)

    #return true, if the common prefix of both is equal to directory
    #e.g. /a/b/c/d.rst and directory is /a/b, the common prefix is /a/b
    return os.path.commonprefix([file, directory]) == directory

def getScriptFolder():
	import inspect
	""" Get the directory of the script is calling this function """
	return os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe().f_back))) 

def mkdir(*path):
	res = toPath(*path)	
	if not os.path.isdir(res):
		os.mkdir(res)
	return res

def partition(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i+n]

def pad_infinite(iterable, padding=None):
   return itertools.chain(iterable, itertools.repeat(padding))

def pad(iterable, size, padding=None):
   return itertools.islice(pad_infinite(iterable, padding), size)

def makeSlice(string):
	def toInt(s):
		try:
			return int(s)
		except ValueError:
			return None

	return slice(*list(pad(map(toInt, string.split(":")), 3)))

def dateToString(date):
	return date.strftime("%Y-%m-%dT%H:%M:%S.%f")

def stringToDate(string):
	return datetime.datetime.strptime(string, "%Y-%m-%dT%H:%M:%S.%f" )

def safeget(dct, *keys, failure = None):
    for key in keys:
        if key in dct.keys():
            dct = dct[key]
        else: 
            return failure
    return dct

def find_pyconfig(path):
	while path != "":
		if os.path.exists(toPath(path, "pyconfig.ini")): 
			return toPath(path, "pyconfig.ini")
		else:
			path = os.path.split(path)[0];
	return None

def stats(l):
	mean = sum(l)/len(l)
	std = math.sqrt(sum([pow(mean-x,2) for x in l])/len(l))
	return mean, std