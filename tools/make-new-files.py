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
 
from __future__ import print_function

import os
import sys
import argparse
import re
import subprocess


if os.name == 'posix':
	CMAKE='cmake'
else:
	CMAKE='cmake.exe'

try:
	import colorama
	colorama.init()
	
	def print_error(mess):
		print(colorama.Fore.RED + colorama.Style.BRIGHT + mess + colorama.Style.RESET_ALL)
	def print_warn(mess):
		print(colorama.Fore.RED + mess + colorama.Style.RESET_ALL)	
	
except ImportError:
	def print_error(mess):
		print(mess)
	def print_warn(mess):
		print(mess)	 		

parser = argparse.ArgumentParser(description='Add new files to Inviwo.\n typical usage: \n python.exe ./make-new-files.py --svn --cmake ../build ../include/inviwo/path/to/h-file/MyNewClass', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('names', type=str, nargs='+', action="store", help='Classes to add, form: path/to/h-file/NewClassName')
parser.add_argument("--dummy", action="store_true", dest="dummy", help="Write local testfiles instead")
parser.add_argument("--force", action="store_true", dest="force", help="Overwrite exsting files")
parser.add_argument("--cmake", type=str, nargs=1, action="store", dest="builddir", help="Rerun cmake in the specified build directory", default="")
parser.add_argument("--no-header", action="store_false", default=True, dest="header", help="Don't write header file")
parser.add_argument("--no-source", action="store_false", default=True, dest="source", help="Don't write source file")
parser.add_argument("--frag", action="store_true", dest="frag", help="Add fragment shader")
parser.add_argument("--vert", action="store_true", dest="vert", help="Add vertex shader")
parser.add_argument("--processor", action="store_true", dest="processor", default=False, help="Make a skeleton inviwo processor")

def find_inv_path():
	path = os.path.abspath(sys.argv[0])
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
		if (os.path.exists(os.sep.join(folders[:i] + ['modules', 'base'])) 
		and os.path.exists(os.sep.join(folders[:i] + ['include', 'inviwo']))
		and os.path.exists(os.sep.join(folders[:i] + ['tools', 'templates']))):
			basepath = os.sep.join(folders[:i])
			break

	return basepath

def make_template(file, name, define, api, incfile, author = "<Author>" ):
	lines = []
	with open(file,'r') as f:
		for line in f:
			line = line.replace("<name>", name)
			line = line.replace("<lname>", name.lower())
			line = line.replace("<uname>", name.upper())
			line = line.replace("<api>", api)
			line = line.replace("<define>", define)
			line = line.replace("<incfile>", incfile)
			line = line.replace("<Author>", author)
			lines.append(line)
		return "".join(lines)


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
	


def find_cmake_file(path):
	pathlist = path.split(os.sep)
	for i in range(len(pathlist),0,-1):
		if os.path.exists(os.sep.join(pathlist[:i] + ["CMakeLists.txt"])):
			return os.sep.join(pathlist[:i] + ["CMakeLists.txt"])
	return []
	
def make_path_relative(path, base):
    (basedir, file)  = os.path.split(base)
    return os.path.relpath(path, basedir)
	
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
			self.cmake_source = "/".join(make_path_relative(self.source, self.cmake_file).split(os.sep))
				
		elif re.compile(r".*/modules/.*").match("/".join(abspath)): # Module path
			module_pos = abspath.index("modules")
			self.module_name = abspath[module_pos+1]
			self.api_def = "IVW_MODULE_" + self.module_name.upper() + "_API"
			self.module_define = "<modules/" + self.module_name + "/" + self.module_name + "moduledefine.h>"
			self.include_define = "\"" + self.file_name + ".h\""
			self.header_file = os.sep.join(abspath + [self.file_name + ".h"])
			self.source = os.sep.join(abspath + [self.file_name])
			self.cmake_file = find_cmake_file(self.source)
			self.cmake_header_file = "${CMAKE_CURRENT_SOURCE_DIR}/"  + "/".join(make_path_relative(self.header_file, self.cmake_file).split(os.sep))
			self.cmake_source = "${CMAKE_CURRENT_SOURCE_DIR}/"  + "/".join(make_path_relative(self.source, self.cmake_file).split(os.sep))
		
	def get_source_file(self, ext = ".cpp"):
		return self.source + ext
	
	def get_cmake_source(self,ext = ".cpp"):
		return self.cmake_source + ext
	
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
	    

def write_file(paths, template, file, comment, force=False):
	if os.path.exists(file) and not force:
		print_error("... File exists: " + file + ", use --force or overwrite")
		return
	elif os.path.exists(file) and force:
		print_warn("... Overwriting existing file")
	
	with open(file, "w") as f:	
		print(comment + f.name)
		f.write(make_template(template, paths.class_name, paths.module_define,  paths.api_def, paths.include_define))

		
print("Adding files to inwivo")

args = parser.parse_args()
ivwpath = find_inv_path()
templates = os.sep.join([ivwpath, 'tools', 'templates'])
	
for name in args.names:
	paths = Paths(name)
	paths.info()
		
	cmakefile = CMakefile(paths.cmake_file)
		
	if args.header:
		cmakefile.add_file("HEADER_FILES", paths.cmake_header_file)
		write_file(paths,
				   os.sep.join([templates, "processor.h" if args.processor else "file.h"]), 	
			       paths.file_name + ".h" if args.dummy else paths.header_file, 
			       "... Writing header file: ", args.force)

	if args.source:
		cmakefile.add_file("SOURCE_FILES",  paths.get_cmake_source())
		write_file(paths,
				   os.sep.join([templates, "processor.cpp" if args.processor else "file.cpp"]), 	
			       paths.file_name + ".cpp" if args.dummy else paths.get_source_file(), 
			       "... Writing source file: ", args.force)

	if args.frag:
		cmakefile.add_file("SHADER_FILES",  paths.get_cmake_source(".frag"))
		write_file(paths,
				   os.sep.join([templates, "fragment.frag"]), 	
			       paths.file_name + ".frag" if args.dummy else paths.get_source_file(".frag"), 
			       "... Writing fragment file: ", args.force)
			
	if args.vert:
		cmakefile.add_file("SHADER_FILES",  paths.get_cmake_source(".vert"))
		write_file(paths,
				   os.sep.join([templates, "vertex.vert"]), 	
			       paths.file_name + ".vert" if args.dummy else paths.get_source_file(".vert"), 
			       "... Writing vertex file: ", args.force)						 

	cmakefile.write("CMakefile.dummy.txt" if args.dummy else paths.cmake_file)
	
	
if args.builddir != "":
	print("Running cmake:")
	try:
		with subprocess.Popen([CMAKE, str(args.builddir[0])], 
								stdout=subprocess.PIPE, 
								stderr=subprocess.STDOUT,
								universal_newlines=True) as proc:
	
			for line in proc.stdout:
				print(line, end='', flush=True)
	except FileNotFoundError:
		print_error("... Could not find " + CMAKE + " in the path")
		






