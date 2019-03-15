import sys 
import os
import subprocess
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = [
	"C:/Users/petst55.AD/Documents/Inviwo/inviwo"
]

excludespatterns = ["*/ext/*", "*/templates/*", "*/tools/codegen/*" , "*moc_*", "*cmake*"];

files = refactoring.find_files(paths, ['*.h', '*.hpp', '*.cpp'], excludes=excludespatterns)

for file in files:
	print("check " + file)
	with subprocess.Popen(["clang-format.exe", "-i", file], 
		stdout=subprocess.PIPE, 
		stderr=subprocess.STDOUT,
		universal_newlines=True) as proc:
		for line in proc.stdout:
			print(line, end='', flush=True)