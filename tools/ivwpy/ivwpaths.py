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
from . import util

class IvwPaths:
	""" 
	Given a path to a file try and find all related files.
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
			self.cmake_file = self.find_cmake_file(self.source)
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


def test_for_inviwo(path):
	return (os.path.exists(os.sep.join([path] + ['modules', 'base'])) 
		and os.path.exists(os.sep.join([path] + ['include', 'inviwo']))
		and os.path.exists(os.sep.join([path] + ['tools', 'templates'])))

def find_inv_path():
	path = util.getScriptFolder()
	folders=[]
	while 1:
		path, folder = os.path.split(path)
		if folder != "":
			folders.append(folder)
		else:
			if path != "":
				folders.append(path)
			break

	folders.reverse()
	
	basepath = ""
	for i in range(len(folders), 0 ,-1):
		if test_for_inviwo(os.sep.join(folders[:i])):
			basepath = os.sep.join(folders[:i])
			break

	return basepath


