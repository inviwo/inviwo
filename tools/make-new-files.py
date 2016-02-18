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

def make_template(file, name, define, api, incfile, author = "<Author>" ):
	lines = []
	with open(file,'r') as f:
		for line in f:
			line = line.replace("<name>", name)
			line = line.replace("<dname>", re.sub("([a-z])([A-Z])","\g<1> \g<2>", name))
			line = line.replace("<lname>", name.lower())
			line = line.replace("<uname>", name.upper())
			line = line.replace("<api>", api)
			line = line.replace("<define>", define)
			line = line.replace("<incfile>", incfile)
			line = line.replace("<Author>", author)
			lines.append(line)
		return "".join(lines)
	
    
def write_file(paths, template, file, comment, force=False):
	(path, filename)  = os.path.split(file)
	ivwpy.util.mkdir(path)

	if os.path.exists(file) and not force:
		cp.print_error("... File exists: " + file + ", use --force or overwrite")
		return
	elif os.path.exists(file) and force:
		cp.print_warn("... Overwriting existing file")
	
	with open(file, "w") as f:	
		print(comment + f.name)
		f.write(make_template(template, paths.class_name, paths.module_define,  paths.api_def, paths.include_define))

	
if __name__ == '__main__':
	parser = argparse.ArgumentParser(
		description="Add new files to Inviwo. typical usage: python.exe ./make-new-files.py --cmake ../build "
		 			+ "../modules/mymodule/path/to/h-file/MyNewClass",
		formatter_class=argparse.ArgumentDefaultsHelpFormatter
	)
	parser.add_argument('names', type=str, nargs='+', action="store", 
						help='Classes to add, form: path/to/h-file/NewClassName' 
							+' Note: the path should be to where the header' 
		     				+ ' should be even if you do not genereate a header')
	parser.add_argument("-p", "--processor", action="store_true", dest="processor", default=False, 
						help="Make a skeleton inviwo processor")
	parser.add_argument("-i", "--inviwo", type=str, dest="ivwpath", 
						help="Path to the inviwo repository. If now given the script tries to find it in the current path")
	parser.add_argument("-c", "--cmake", type=str, nargs=1, action="store", dest="builddir", 
						help="Rerun CMake in the specified build directory")

	parser.add_argument("-nh", "--no-header", action="store_true", dest="header", 
						help="Don't add header file")
	parser.add_argument("-ns", "--no-source", action="store_true", dest="source", 
						help="Don't add source file")
	parser.add_argument("-f", "--frag", action="store_true", dest="frag", 
						help="Add fragment shader file")
	parser.add_argument("-v", "--vert", action="store_true", dest="vert", 
						help="Add vertex shader file")
	parser.add_argument("-g", "--geom", action="store_true", dest="geom", 
						help="Add geometry shader file")

	parser.add_argument("-d", "--dummy", action="store_true", dest="dummy", 
						help="Write local testfiles instead")
	parser.add_argument("--force", action="store_true", dest="force", 
						help="Overwrite exsting files")

	args = parser.parse_args()

	cp.print_warn("Adding files to inwivo")
	ivwpath = ivwpy.ivwpaths.find_inv_path() if args.ivwpath == None else args.ivwpath

	if not ivwpy.ivwpaths.test_for_inviwo(ivwpath):
		cp.print_error("Error could not find the inviwo repository, use --inviwo to specify where the inviwo repository is.")
		sys.exit(1)

	templates = os.sep.join([ivwpath, 'tools', 'templates'])
		
	for name in args.names:
		paths = ivwpy.ivwpaths.IvwPaths(name)
		paths.info()
			
		cmakefile = ivwpy.cmake.CMakefile(paths.cmake_file)
			
		if not args.header:
			cmakefile.add_file("HEADER_FILES", paths.cmake_header_file)
			write_file(paths,
					   os.sep.join([templates, "processor.h" if args.processor else "file.h"]), 	
				       paths.file_name + ".h" if args.dummy else paths.header_file, 
				       "... Writing header file: ", args.force)

		if not args.source:
			cmakefile.add_file("SOURCE_FILES",  paths.get_cmake_source())
			write_file(paths,
					   os.sep.join([templates, "processor.cpp" if args.processor else "file.cpp"]), 	
				       paths.file_name + ".cpp" if args.dummy else paths.get_source_file(), 
				       "... Writing source file: ", args.force)

		if args.frag:
			cmakefile.add_file("SHADER_FILES",  paths.get_cmake_glsl(".frag"))
			write_file(paths,
					   os.sep.join([templates, "fragment.frag"]), 	
				       paths.file_name + ".frag" if args.dummy else paths.get_glsl_file(".frag"), 
				       "... Writing fragment file: ", args.force)
				
		if args.vert:
			cmakefile.add_file("SHADER_FILES",  paths.get_cmake_glsl(".vert"))
			write_file(paths,
					   os.sep.join([templates, "vertex.vert"]), 	
				       paths.file_name + ".vert" if args.dummy else paths.get_glsl_file(".vert"), 
				       "... Writing vertex file: ", args.force)		

		if args.geom:
			cmakefile.add_file("SHADER_FILES",  paths.get_cmake_glsl(".geom"))
			write_file(paths,
					   os.sep.join([templates, "geometry.geom"]), 	
				       paths.file_name + ".geom" if args.dummy else paths.get_glsl_file(".geom"), 
				       "... Writing geometry file: ", args.force)						 

		cmakefile.write("CMakefile.dummy.txt" if args.dummy else paths.cmake_file)
		
		
	if args.builddir != None:
		ivwpy.cmake.runCMake(str(args.builddir[0]))
	else:
		cp.print_warn("Don't forget to rerun CMake")
	
	if args.processor: cp.print_warn("Don't forget to register the processor in the module")
	cp.print_warn("Done")




