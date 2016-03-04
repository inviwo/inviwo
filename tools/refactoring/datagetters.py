import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

#paths = [
#	"/Users/petst/Work/Projects/Inviwo-Developent/Private/Inviwo-dev", 
#	"/Users/petst/Work/Projects/Inviwo-Developent/Private/Inviwo-research"
#]

paths = [
	"C:/Users/petst55/Work/Inviwo/Inviwo-dev",
	"C:/Users/petst55/Work/Inviwo/Inviwo-research"
]

excludespatterns = ["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*", "*/vrnbase/*"];

files = refactoring.find_files(paths, ['*.h', '*.cpp'], excludes=excludespatterns)

def replace(pattern, replacement) :
	print("Matches:")
	matches = refactoring.find_matches(files, pattern)
	
	print("\n")
	print("Replacing:")
	refactoring.replace_matches(matches, pattern, replacement)

replacements1 = {
	"getValueAsSingleDouble"   : "getAsNormalizedDouble",
	"getValueAsVec2Double"     : "getAsNormalizedDVec2",
	"getValueAsVec3Double"     : "getAsNormalizedDVec3",
	"getValueAsVec4Double"     : "getAsNormalizedDVec4",

	"setValueFromSingleDouble" : "setFromDouble",
	"setValueFromVec2Double"   : "setFromDVec2",
	"setValueFromVec3Double"   : "setFromDVec3",
	"setValueFromVec4Double"   : "setFromDVec4"
 }

print("Looking in " + str(len(files)) + " files")

for k,v in replacements1.items():
	replace(r"\b"+k+r"\b", v)