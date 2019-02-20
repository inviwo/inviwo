import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = [
	"C:/Users/petst55.AD/Documents/Inviwo/inviwo" #,
	#"C:/Users/petst55/Work/Inviwo/Inviwo-research",
	#"C:/Users/petst55/Work/Inviwo/Inviwo-modules"
]

excludespatterns = ["*/ext/*", "*moc_*", "*/proteindocking/*", "*/proteindocking2/*", 
					"*/genetree/*", "*.DS_Store", "*DS_mapp", ".md", "*.suo" , "*.h5",
					"*.jpg", "*.JPG", "*.jpeg", "*.lib", "*.dll", "*.inv", "*.dat", "*.ivf","*.tiff",
					"*.png", "*.ttf", "*.tif", "*.pyc", "*.raw", "*.bmp", "*.wav", "*.xcf", "*.ico", "*.icns",
					"*.qch", "*.qhc", "*.exr", "*.pwm", "*.pvm", "*.pdf", "*.otf", "*.exe", "*.fbx", "*.svg", 
					"*.itf", "*.qrc", "*.md", "*/.git*", "*/.clang-format", "*/LICENSE", ".git", "Jenkinsfile",
					".gitattributes",  "*/AUTHORS", "" "*/tools/meta/templates/*", "*.natvis", "*/depends.cmake", 
					"*moduledefine.h", "*moduledefine.hpp", "*/config.json", "*.js", "*/CMakeLists.txt"]

copyright_replacements  = {
	r"(\s*[*#]\s+Copyright \(c\) 201\d-)201[12345678]( Inviwo Foundation\s*)" : r"\g<1>2019\g<2>",
	r"(\s*[*#]\s+Copyright \(c\) )(201[12345678])( Inviwo Foundation\s*)" : r"\g<1>\g<2>-2019\g<3>"
}

files = refactoring.find_files(paths, ['*'], excludes=excludespatterns)

def replace(pattern, replacement) :
	print("Matches:")
	matches = refactoring.find_matches(files, pattern)
	
	print("\n")
	print("Replacing:")
	refactoring.replace_matches(matches, pattern, replacement)

print("Looking in " + str(len(files)) + " files")

for k,v in copyright_replacements.items():
	replace(k, v)



