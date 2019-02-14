import sys 
import os
import re
from enum import Enum

import refactoring # Note: refactoring.py need to be in the current working directory


## NOTE: update copyright year here
currentYear = 2019

excludespatterns = ["*/ext/*", "*moc_*", "*/proteindocking/*", "*/proteindocking2/*", 
					"*/genetree/*", "*.DS_Store", "*DS_mapp", ".md", "*.suo" , "*.h5",
					"*.jpg", "*.JPG", "*.jpeg", "*.lib", "*.dll", "*.inv", "*.dat", "*.ivf","*.tiff",
					"*.png", "*.ttf", "*.tif", "*.pyc", "*.raw", "*.bmp", "*.wav", "*.xcf", "*.ico", "*.icns",
					"*.qch", "*.qhc", "*.exr", "*.pwm", "*.pvm", "*.pdf", "*.otf", "*.exe", "*.fbx", "*.svg", 
					"*.itf", "*.qrc", "*.md", "*/.git*", "*/.clang-format", "*/LICENSE", ".git", "Jenkinsfile",
					".gitattributes",  "*/AUTHORS", "" "*/tools/meta/templates/*", "*.natvis", "*/depends.cmake", 
					"*moduledefine.h", "*moduledefine.hpp", "*/config.json", "*.js", "*/CMakeLists.txt"]

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

files = refactoring.find_files(paths, ['*'], excludes=excludespatterns)

def replace(pattern, replacement) :
	matches = refactoring.find_matches(files, pattern)
	print(matches.size());

def createYearRegExp():
	if (currentYear <= 2000):
		print_error("Invalid current year: " + str(currentYear))
		sys.exit(-1)
	# regular expression should match any year up to current year
	# the copy right year might consist of a single year, e.g. "2017"
	# or a range of years "2010-2018"
	#
	# The resulting expression for currentYear = 2018 will have the form of:
	#    r"^((?:20[0-1]\d-)?20[0-1][0-7]$)"
	years = (currentYear - 1) % 10
	decades = int((currentYear - 1) / 10) % 10
	decadesStr = "[0-" + str(decades) + "]" if decades > 0 else "0"
	yearsStr = "[0-" + str(years) + "]" if years > 0 else "0"

	return re.compile(r"^((?:20" + decadesStr + r"\d-)?20" + decadesStr + yearsStr + r"$)")

# match Inviwo Foundation copyright line 
copyrightLine_regex = re.compile(r"\s*[*#]\s+Copyright \(c\) (.*?) Inviwo Foundation\s*")
# matches single year or range of years, i.e. '2018' and '2010-2018'
copyrightYear_regex = re.compile(r"^((?:\d\d\d\d-)?\d\d\d\d)$")
# matches only if right-most year is outdated, i.e. < currentYear
copyrightOutdated_regex = createYearRegExp()


class CopyrightState(Enum):
	Correct = 0
	Outdated = 1
	MalformedYear = 2 
	Missing = 3  # or malformed 


def checkline(line):
	linematch = copyrightLine_regex.search(line)
	if linematch:
		yearmatch = copyrightYear_regex.search(linematch.group(1))
		if yearmatch:
			outdated = copyrightOutdated_regex.search(yearmatch.group(1))
			if outdated:
				# copyright is not up to date
				return CopyrightState.Outdated
			else:
				# copyright is correctly formatted and up to date
				return CopyrightState.Correct
		else:
			# malformed year
			return CopyrightState.MalformedYear 
	else:
		# no copyright info / entirely malformed
		return CopyrightState.Missing

def checkfile(filehandle, filename):
	for (i,line) in enumerate(filehandle):
		result = checkline(line)
		if result == CopyrightState.Outdated:
			print_warn("Copyright outdated in " + str(filename))
			print(str(i) + ": " + line.rstrip())
			return 1 # flag copyright error
		elif result == CopyrightState.MalformedYear:
			print_warn("Copyright year malformed in " + str(filename))
			print(str(i) + ": " + line.rstrip())
			print("Expecting either '201x' or '201x-201y'")
			return 1 # flag copyright error
		elif result == CopyrightState.Correct:
			return 0
	# did not find a valid copyright line
	print_warn("Copyright information missing in " + str(filename))
	return 0 # TODO: flag copyright error


def test(file):
	with open(file, 'r', encoding="UTF-8") as f:
		try:
			return checkfile(f, file)
		except UnicodeDecodeError:
			print_warn(file + ": File not utf-8 encoded, fall-back to Western encoding (Windows 1252)")
			with open(file, 'r', encoding="cp1252") as f:
				try:
					return checkfile(f)
				except UnicodeDecodeError:
					print_error("Encoding error: " + file)
	return 0

matches = 0

print("Checking copyright for year " + str(currentYear))
print("Looking in " + str(len(files)) + " files")

for file in files:
	matches=matches+test(file)

sys.exit(matches)


