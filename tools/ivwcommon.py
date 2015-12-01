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
import sys
import re
import inspect
import argparse
import subprocess
import configparser

try:
	import colorama
	colorama.init()	
	def print_error(mess, **kwargs):
		print(colorama.Fore.RED + colorama.Style.BRIGHT + mess + colorama.Style.RESET_ALL, **kwargs)
	def print_warn(mess, **kwargs):
		print(colorama.Fore.YELLOW + colorama.Style.BRIGHT + mess + colorama.Style.RESET_ALL, **kwargs)	
	
except ImportError:
	def print_error(mess, **kwargs):
		print(mess, **kwargs)
	def print_warn(mess, **kwargs):
		print(mess, **kwargs) 		

def getScriptFolder():
	return os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))) # script directory

def findCMake():
	config = configparser.ConfigParser()
	config.read(os.sep.join([getScriptFolder(), "pyconfig.ini"]))
	if config.has_option("CMake", "path"):
		cmake = config.get("CMake", "path")
	elif os.name == 'posix': 
		cmake='cmake'
	else: 
		cmake='cmake.exe'

	return cmake

def runCMake(path, options = []):
	print_warn("Running CMake:")
	cmake = findCMake()
	try:
		with subprocess.Popen([cmake] + options + [path], 
	  						  stdout=subprocess.PIPE, 
							  stderr=subprocess.STDOUT,
							  universal_newlines=True) as proc:
			for line in proc.stdout:
				print(line, end='', flush=True)
	except FileNotFoundError:
		print_error("... Could not find " + cmake + " in the path")

class Paths:
	""" 
	Figure out all the relevent paths and stuff
	"""
	def __init__(self, filepath):
		(path, file)  = os.path.split(filepath)
		abspath = os.path.abspath(path).split(os.sep)
		self.class_name = file
		self.file_name = file.lower()
		
		m = re.compile(r".*/include/inviwo/(?P<kind>core|qt)/((?P<qt>editor|widgets))?.*").match("/".join(abspath))
		if m: # Core/Qt path
			inviwo_pos = next(i for (i,x) in enumerate(abspath) if x=="inviwo" and abspath[i-1]=="include")
			self.module_name = m.group("kind")
			if self.module_name == "qt": self.module_name += m.group("qt")
			self.api_def = "IVW_" + self.module_name.upper() + "_API"
			if m.group("kind") == "qt":
				self.module_define = "<inviwo/qt/"+ m.group("qt") +"/inviwo" + self.module_name.lower() + "define.h>"
			else:
				self.module_define = "<inviwo/core/common/inviwo" + self.module_name.lower() + "define.h>"
			
			self.include_define = "<" + "/".join(abspath[inviwo_pos:] + [self.file_name + ".h"]) + ">"
			self.header_file = os.sep.join(abspath + [self.file_name + ".h"])
			self.source = os.sep.join(abspath[:inviwo_pos-1] + ["src"] + abspath[inviwo_pos+1:] + [self.file_name])
			self.cmake_file = find_cmake_file(self.source)
			self.cmake_header_file = "/".join(["${IVW_INCLUDE_DIR}"] + abspath[inviwo_pos:] + [file.lower() +".h"])
			self.cmake_source = "/".join(self.make_path_relative(self.source, self.cmake_file).split(os.sep))
				
		elif re.compile(r".*/modules/.*").match("/".join(abspath)): # Module path
			module_pos = abspath.index("modules")
			self.module_name = abspath[module_pos+1]
			self.api_def = "IVW_MODULE_" + self.module_name.upper() + "_API"
			self.module_define = "<modules/" + self.module_name + "/" + self.module_name + "moduledefine.h>"
			self.include_define = "\"" + self.file_name + ".h\""
			self.header_file = os.sep.join(abspath + [self.file_name + ".h"])
			self.source = os.sep.join(abspath + [self.file_name])
			self.cmake_file = self.find_cmake_file(self.source)
			self.cmake_header_file = "${CMAKE_CURRENT_SOURCE_DIR}/"  + "/".join(self.make_path_relative(self.header_file, self.cmake_file).split(os.sep))
			self.cmake_source = "${CMAKE_CURRENT_SOURCE_DIR}/"  + "/".join(self.make_path_relative(self.source, self.cmake_file).split(os.sep))
		
	def get_source_file(self, ext = ".cpp"):
		return self.source + ext
	
	def get_cmake_source(self,ext = ".cpp"):
		return self.cmake_source + ext
	
	def make_path_relative(self, path, base):
		(basedir, filename) = os.path.split(base)
		return os.path.relpath(path, basedir)

	def find_cmake_file(self, path):
		pathlist = path.split(os.sep)
		for i in range(len(pathlist),0,-1):
			if os.path.exists(os.sep.join(pathlist[:i] + ["CMakeLists.txt"])):
				return os.sep.join(pathlist[:i] + ["CMakeLists.txt"])
		return []

	def info(self):
		print("Class name:       " + self.class_name)
		print("... File name:    " + self.file_name)
		print("... Module name:  " + self.module_name)
		print("... API:          " + self.api_def)
		print("... Module def:   " + self.module_define)
		print("... Include def:  " + self.include_define)
		print("... Header file:  " + self.header_file)
		print("... Source file:  " + self.get_source_file())
		print("... CMake file:   " + self.cmake_file)
		print("... CMake header: " + self.cmake_header_file)
		print("... CMake source: " + self.get_cmake_source())


class CMakefile:
	"""
	Represent a cmake file
	"""
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