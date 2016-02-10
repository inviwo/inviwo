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
import re
import configparser
import subprocess

from . import util
from . import colorprint as cp
from . import ivwpaths

def findCMake(pyconf = ""):
	config = configparser.ConfigParser()
	config.read([
		util.toPath(ivwpaths.find_inv_path(), "pyconfig.ini"),
		pyconf
		])
	if config.has_option("CMake", "path"):
		cmake = config.get("CMake", "path")
	elif os.name == 'posix': 
		cmake='cmake'
	else: 
		cmake='cmake.exe'

	return cmake

def runCMake(path, options = []):
	cp.print_warn("Running CMake:")
	cmake = findCMake()
	try:
		with subprocess.Popen([cmake] + options + [path], 
	  						  stdout=subprocess.PIPE, 
							  stderr=subprocess.STDOUT,
							  universal_newlines=True) as proc:
			for line in proc.stdout:
				print(line, end='', flush=True)
	except FileNotFoundError:
		cp.print_error("... Could not find " + cmake + " in the path")


class CMakefile:
	"""Represent a cmake file"""
	def __init__(self, filepath):
		self.filepath = filepath
		self.lines = []
		with open(filepath, "r") as f:
			for l in f:
				self.lines.append(l)
	
	def add_file(self, group, file):
		m1 = re.compile(r"\s*set\(" + group + "\s*")
		m0 = re.compile(r"\s*\)\s*")
		it = iter(self.lines)
		self.found_group = False
		def sort_and_insert_line(f, line):
			lines = []
			for l in f:
				if m0.match(l):
					self.found_group = True
					lines.append("    " + line + "\n")
					lines.sort()
					
					# remove duplicates
					seen = set()
					lines = [x for x in lines if not ( x in seen or seen.add(x))]
					
					lines.append(l)
					break
				elif l.strip() != "":	
					lines.append(l.replace("\t","    "))
				
			return lines
		
		lines = []
		for line in it:
			if m1.match(line):
				lines.append(line)
				lines.extend(sort_and_insert_line(it, file))						
			else:
				lines.append(line)
			
		if not self.found_group:
			print_error("... Could not find group " + group + " in cmakelist: " + self.filepath)
		self.lines = lines
	
	def write(self, file = ""):
		with open(file if file != "" else self.filepath, "w") as f:
			print("... Updating cmakelists: " + f.name)
			for l in self.lines:
				f.write(l)