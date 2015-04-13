import fnmatch
import os
import fileinput
import re
import codecs

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

def exclude_name(name, filters):
	for filter in filters:
		if fnmatch.fnmatch(name, filter):
			return True
	return False


def find_files(paths, extensions, excludes=[""]):
	matches = []
	for path in paths:
		for root, dirnames, filenames in os.walk(path):
			dirnames[:] = [d for d in dirnames if d not in [".git"]]
			for extension in extensions:
				for filename in fnmatch.filter(filenames, extension):
					if not exclude_name(root + os.sep + filename, excludes):
						matches.append(os.path.join(root, filename))
	return matches

def find_matches(files, expr):
	r = re.compile(r"(" + expr +")")
	for file in files:
		match_in_file = False
		with fileinput.input(file) as f:
			for (i,line) in enumerate(f):
				if r.search(line):
					if not match_in_file: 
						print_warn("Match in: " + file)
						match_in_file = True
					matched = r.sub(colorama.Fore.YELLOW+ r"\1"+ colorama.Style.RESET_ALL, line.rstrip())
					print("{0:5d} {1:s}".format(i,matched))

def replace_matches(files, expr, repl, dummy=False):
	r = re.compile(r"(" + expr +")")
	for file in files:
		match_in_file = False
		
		with open(file, "r") as f:
			lines = f.readlines()
		
		if not dummy:
			with open(file, "w") as f:
				for (i,line) in enumerate(lines):
					if r.search(line):
						if not match_in_file: 
							print_warn("Match in: " + file)
							match_in_file = True
						matched = r.sub(colorama.Fore.YELLOW+ r"\1"+ colorama.Style.RESET_ALL, line.rstrip())
						replaced = r.sub(colorama.Fore.RED + repl + colorama.Style.RESET_ALL, line.rstrip())
						f.write(r.sub(repl, line))
						print("- {0:5d} {1:s}".format(i,matched))
						print("+ {0:5d} {1:s}".format(i,replaced))
					else:
						f.write(line)
		else: 
			for (i,line) in enumerate(lines):
				if r.search(line):
					if not match_in_file: 
						print_warn("Match in: " + file)
						match_in_file = True
					matched = r.sub(colorama.Fore.YELLOW+ r"\1"+ colorama.Style.RESET_ALL, line.rstrip())
					replaced = r.sub(colorama.Fore.RED + repl + colorama.Style.RESET_ALL, line.rstrip())
					print("- {0:5d} {1:s}".format(i,matched))
					print("+ {0:5d} {1:s}".format(i,replaced))


def check_file_type(files, enc):
	matches = []
	for f in files:
		try:
			fh = codecs.open(f, 'r', encoding=enc)
			fh.readlines()
			fh.seek(0)
		except UnicodeDecodeError:
			print(f)
			matches.append(f)

	return matches

def convert_file(file, enc):
	with open(file, 'br') as f:
		buff = f.read()
	text = buff.decode(enc, errors='replace')
	with open(file, 'w') as f:
		f.write(text)


source_extensions = ('*.cpp', '*.h')

# examples
# in ipython "run tools/replace-in-files.py"
# n = find_files(["."], source_extensions, excludes=["*ext*"])
# n = find_files(["inviwo-dev", "inviwo-research"], source_extensions, excludes=["*ext*", "*moc*", "*cmake*"])
# n = find_files(["inviwo-dev", "inviwo-research"], source_extensions, excludes=["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*"])
# replace_matches(n, r"^(.*)ProcessorClassIdentifier\((\S+)\s*,\s*(.+)\);?\W*$", r"\1\2ProcessorClassVersion(\3, 0);\n")


