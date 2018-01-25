import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = [
	"d:/projects/inviwo/inviwo-dev"
#	"/Users/petst/Work/Projects/Inviwo-Developent/Private/Inviwo-research"
]

#paths = [
#	"C:/Users/petst55/Work/Inviwo/Inviwo-dev",
#	"C:/Users/petst55/Work/Inviwo/Inviwo-research"
#]

excludespatterns = ["*/ext/*", "*moc_*", "*/proteindocking/*", "*/proteindocking2/*", 
					"*/genetree/*", "*.DS_Store", "*DS_mapp",
					"*.jpg", "*.JPG", "*.jpeg", "*.lib", "*.dll",
					"*.png", "*.ttf", "*.tif", "*.pyc", "*.raw", "*.bmp", "*.wav", "*.ico", "*.icns",
					"*.qch", "*.qhc", "*.exr", "*.pwm", "*.pvm", "*.pdf", "*.otf", "*.exe"]

files = refactoring.find_files(paths, ['*'], excludes=excludespatterns)

def replace(pattern, replacement) :
	print("Matches:")
	matches = refactoring.find_matches(files, pattern)
	
	print("\n")
	print("Replacing:")
	refactoring.replace_matches(matches, pattern, replacement)


copyright_replacements  = {
	r"(\s*[*#]\s+Copyright \(c\) 201\d-)201[1234567]( Inviwo Foundation\s*)" : r"\g<1>2018\g<2>",
	r"(\s*[*#]\s+Copyright \(c\) )(201[1234567])( Inviwo Foundation\s*)" : r"\g<1>\g<2>-2018\g<3>"
}

print("Looking in " + str(len(files)) + " files")

for k,v in copyright_replacements.items():
	replace(k, v)



