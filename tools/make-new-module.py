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
import argparse
import re
import subprocess
import sys

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


def test_for_inviwo(path):
	return (os.path.exists(os.sep.join([path] + ['modules', 'base'])) 
		and os.path.exists(os.sep.join([path] + ['include', 'inviwo']))
		and os.path.exists(os.sep.join([path] + ['tools', 'templates'])))


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
		if test_for_inviwo(os.sep.join(folders[:i])):
			basepath = os.sep.join(folders[:i])
			break

	return basepath


def make_module(ivwpath, path, name, verbose, dummy):
	if os.path.exists(os.sep.join([path, name])):
		print("Error module: "+ name + ", already exits")
		return
	
	print("Make module: " + name)
	uname = name.upper()
	lname = name.lower()
	
	files = ["CMakeLists.txt", "depends.cmake", "module.cpp", "module.h", "moduledefine.h"]
	prefixes = ["", "", lname, lname, lname]
	
	module_dir = os.sep.join([path, lname])
	
	if not dummy:
		print("... Crate dir: " + module_dir)
		os.mkdir(module_dir)
	
	for prefix, file in zip(prefixes, files):
		outfile = os.sep.join([module_dir, prefix+file])
		lines = []
		with open(os.sep.join([ivwpath, 'tools', 'templates', file]),'r') as f:
			if verbose:
				print("")
				print("FILE: " + os.sep.join([module_dir, prefix+file]))
				print("#"*60)
				
			for line in f:
				line = line.replace("<name>", name)
				line = line.replace("<lname>", lname)
				line = line.replace("<uname>", uname)
				lines.append(line)
				if verbose:
					print(line, end='')
			
			if verbose:
				print("")
		
		if not dummy:
			print("... Write file: " + outfile)
			with open(outfile,'w') as f:
				for line in lines:
					f.write(line)
					
	print("... Done")

				
if __name__ == '__main__':
	if os.name == 'posix': CMAKE='cmake'
	else: CMAKE='cmake.exe'

	parser = argparse.ArgumentParser(description='Add new modules to inviwo', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('modules', type=str, nargs='+', action="store", help='Modules to add, form: path/name1 path/name2 ...')
	parser.add_argument("--dummy", action="store_true", dest="dummy", help="Don't write actual files")
	parser.add_argument("--verbose", action="store_true", dest="verbose", help="Print extra information")
	parser.add_argument("--inviwo", type=str, default="", dest="ivwpath", help="Path to the inviwo repository. Tries to find it in the current path")
	args = parser.parse_args()

	if args.ivwpath == "":
		ivwpath = find_inv_path()
	else:
		ivwpath = args.ivwpath

	if not test_for_inviwo(ivwpath):
		print_error("Error could not find the inviwo repository")
		parser.print_help()
		sys.exit(1)
	
	print("Path to inviwo: " + ivwpath)
		
	for pathname in args.modules:
		path, name = os.path.split(pathname)
		if path == "": path = "."
		make_module(ivwpath, path, name, args.verbose, args.dummy)
		

