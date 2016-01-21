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
import argparse

import ivwpy.colorprint as cp
import ivwpy.util
import ivwpy.ivwpaths
import ivwpy.cmake

def make_module(ivwpath, path, name, verbose, dummy):
	if os.path.exists(os.sep.join([path, name])):
		cp.print_error("Error module: "+ name + ", already exits")
		return
	
	cp.print_warn("Create module: " + name)
	uname = name.upper()
	lname = name.lower()
	
	dirs = [
		{
			"path" : ["data"], 
			 "desc" : "Folder for non code stuff"
		}, 
		{
			"path" : ["data", "image"], 
		 	"desc" : "Image resources"
		}, 
		{
			"path" : ["data", "volumes"], 
		 	"desc" : "Volume resources "
		}, 
		{
			"path" : ["data", "workspaces"], 
		 	"desc" : "Workspaces, listed in File::Examples::ExampleModule"
		}, 
		{
			"path" : ["docs"], 
		 	"desc" : "Put documentation here"
		}, 
		{
			"path" : ["docs", "images"], 
		 	"desc" : "Put images that should show up in doxygen here"
		}, 
		{
			"path" : ["processors"], 
		 	"desc" : "Put processors here"
		},
		{
			"path" : ["properties"], 
		 	"desc" : "Put properties here"
		},
		{
			"path" : ["tests"], 
		 	"desc" : "Test related things"
		},
		{
			"path" : ["tests", "unittests"], 
		 	"desc" : "Put unittests here"
		},
		{
			"path" : ["tests", "regression"], 
		 	"desc" : "Regression Test workspaces, listed in File::Test::ExampleModule." 
		 			 + " Automatically run in regression tests on Jenkins"
		}
	]

	templates = [
		{"file" : "CMakeLists.txt" , "prefix" : ""    , "desc" : "CMake project definition"},
		{"file" : "depends.cmake"  , "prefix" : ""    , "desc" : "List of dependencies to other modules / cmake packages"},
		{"file" : "module.cpp"     , "prefix" : lname , "desc" : "For module registration"},
		{"file" : "module.h"       , "prefix" : lname , "desc" : "For module registration"},
		{"file" : "moduledefine.h" , "prefix" : lname , "desc" : "declspec defines"},
		{"file" : "readme.md"      , "prefix" : ""    , "desc" : "Description of the module, used by CMake"}
	]
	
	module_dir = os.sep.join([path, lname])
	
	print("... Create module dir: " + module_dir)
	if not dummy: os.mkdir(module_dir)
	for dir in dirs:
		print("... Create dir:  {0:.<30} {desc:<100}".format(os.sep.join([lname] + dir["path"])+" ", **dir))
		if not dummy: os.mkdir(os.sep.join([module_dir] + dir["path"]))
	
	for template in templates:
		try:
			with open(os.sep.join([ivwpath, 'tools', 'templates', template["file"]]),'r') as f:
				if verbose:
					print("")
					print("FILE: " + os.sep.join([module_dir, template["prefix"] + template["file"]]))
					print("#"*60)
					
				lines = []
				for line in f:
					line = line.replace("<name>", name)
					line = line.replace("<lname>", lname)
					line = line.replace("<uname>", uname)
					lines.append(line)
					if verbose: print(line, end='')
				
				if verbose: print("")
			
				print("... Create file: {0:.<30} {desc:<100}".format(os.sep.join([lname, template["prefix"] + template["file"]])+" ", **template))	
				if not dummy:
					with open(os.sep.join([module_dir, template["prefix"] + template["file"]]),'w') as f:
						for line in lines:
							f.write(line)
						if verbose: print(line)
	
		except FileNotFoundError as err:
			cp.print_error(err)
			return

					
	print("... Done")

				
if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Add new modules to inviwo', 
									 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('modules', type=str, nargs='+', action="store", 
		                help='Modules to add, form: path/name1 path/name2 ...')
	parser.add_argument("-d", "--dummy", action="store_true", dest="dummy", 
		                help="Don't write actual files")
	parser.add_argument("-v", "--verbose", action="store_true", dest="verbose", 
		                help="Print extra information")
	parser.add_argument("-i", "--inviwo", type=str, default="", dest="ivwpath", 
		                help="Path to the inviwo repository. Tries to find it in the current path")
	parser.add_argument("-c", "--cmake", type=str, nargs=1, action="store", dest="builddir", 
						help="Rerun CMake in the specified build directory")
	args = parser.parse_args()

	if args.ivwpath == "":
		ivwpath = ivwpy.ivwpaths.find_inv_path()
	else:
		ivwpath = args.ivwpath

	if not ivwpy.ivwpaths.test_for_inviwo(ivwpath):
		cp.print_error("Error could not find the inviwo repository")
		parser.print_help()
		sys.exit(1)
	
	print("Path to inviwo: " + ivwpath)
		
	for pathname in args.modules:
		path, name = os.path.split(pathname)
		if path == "": path = "."
		make_module(ivwpath, path, name, args.verbose, args.dummy)

	if args.builddir != None:
		ivwpy.cmake.runCMake(str(args.builddir[0]), ["-DIVW_MODULE_"+name.upper()+"=1"])
	else: 
		cp.print_warn("Don't forget to rerun CMake with -DIVW_MODULE_"+name.upper()+ " to add the module")

	cp.print_warn("Done")