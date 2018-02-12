import sys 
import os
import re
import codecs
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

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

paths = sys.argv[1:]


excludespatterns = ["*/ext/*", "*moc_*", "*/proteindocking/*", "*/proteindocking2/*", 
					"*/genetree/*", "*.DS_Store", "*DS_mapp", ".md", "*.suo" , "*.h5",
					"*.jpg", "*.JPG", "*.jpeg", "*.lib", "*.dll", "*.inv",
					"*.png", "*.ttf", "*.tif", "*.pyc", "*.raw", "*.bmp", "*.wav", "*.xcf", "*.ico", "*.icns",
					"*.qch", "*.qhc", "*.exr", "*.pwm", "*.pvm", "*.pdf", "*.otf", "*.exe", "*.fbx"]

files = refactoring.find_files(paths, ['*'], excludes=excludespatterns)

def replace(pattern, replacement) :
	matches = refactoring.find_matches(files, pattern)
	print(matches.size());


copyright_regex = [
	re.compile(r"(\s*[*#]\s+Copyright \(c\) 201\d-)201[1234567]( Inviwo Foundation\s*)") , 
	re.compile(r"(\s*[*#]\s+Copyright \(c\) )(201[1234567])( Inviwo Foundation\s*)")
]

print("Looking in " + str(len(files)) + " files")



def test(file):
	for r in copyright_regex:
		with codecs.open(file, 'r', encoding="UTF-8") as f:
			try:
				for (i,line) in enumerate(f):
					match = r.search(line) 
					if match:
						print_warn("Incorrect copyright year in " + str(file))
						print(line)
						return 1
			except UnicodeDecodeError:
				print_error("Encoding error: " + file)
	return 0

matches = 0

for file in files:
	matches=matches+test(file)

sys.exit(matches)


