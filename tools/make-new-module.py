from __future__ import print_function

import os
import argparse
import re
import subprocess
import sys

def find_inv_path():
    path = os.path.abspath(sys.argv[0])
    print(path)
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
		print("    Crate dir: " + module_dir)
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
			print("    Write file: " + outfile)
			with open(outfile,'w') as f:
				for line in lines:
					f.write(line)
					
	print("    Done")

				
if __name__ == '__main__':
	if os.name == 'posix':
		CMAKE='cmake'
	else:
		CMAKE='cmake.exe'

	ivwpath = find_inv_path()

	if ivwpath == "":
		print("Error could not find inviwo")
		sys.exit(1)
	
	print("Path to inviwo: " + ivwpath)
	
		
	parser = argparse.ArgumentParser(description='Add new modules to inviwo', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('modules', type=str, nargs='+', action="store", help='Modules to add, form: path/name1 path/name2 ...')
	parser.add_argument("--dummy", action="store_true", dest="dummy", help="Don't write actual files")
	parser.add_argument("--verbose", action="store_true", dest="verbose", help="Print extra information")

	args = parser.parse_args()
	
		
	for pathname in args.modules:
		path, name = os.path.split(pathname)
		make_module(ivwpath, path, name, args.verbose, args.dummy)
		

