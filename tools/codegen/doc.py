
import re
import os
import sys
import fileinput

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

# pwd = C:\Users\petst55\Work\Inviwo
# run Inviwo-dev/tools/replace-in-files.py
# run Inviwo-dev/tools/doc.py
# n = find_files(["inviwo-dev", "inviwo-research"], source_extensions, excludes=["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*", "*/vrnbase/*"])
# m = find_matches(n, "public (Processor|ImageGLProcessor|DataSource<.*>|VolumeGLProcessor|CompositeProcessorGL) ")



def gen(hfile):
	cppfile = hfile.split(".")[0]+".cpp"
	cppfile = re.sub(r"\\include\\inviwo\\", r"\\src\\", cppfile)

	if not os.path.isfile(cppfile): cppfile = hfile

	inports = {}
	outports = {}
	properties = {}

	inr = re.compile(r"^\s*\w*[iI]n[pP]ort\w* (\w*);.*$")
	onr = re.compile(r"^\s*\w*[oO]ut[pP]ort\w* (\w*);.*$")
	pr = re.compile(r"^\s*\w*Property\w* (\w*);.*$")

	has_docpage = False
	dpr = re.compile(r".*\\docpage.*")

	with fileinput.input(hfile) as f:
		for (i,line) in enumerate(f):
			ipm = inr.search(line)
			if ipm:
				inports[ipm.group(1)] = {"hline" : i}
			opm = onr.search(line)
			if opm:
				outports[opm.group(1)] = {"hline" : i}
			pm = pr.search(line)
			if pm:
				properties[pm.group(1)] = {"hline" : i}
			dpm = dpr.search(line)
			if dpm:
				has_docpage = True

	def get_property_init(member, i, line) :
		match = re.search(r'\s*,\s*' + member + r'\("(\w+)"\s*,\s*"(.*)".*', line)
		if(match):
			return {"id" : match.group(1), "name" : match.group(2), "sline" : i}
		else:
			return {}

	def get_port_init(member, i, line) :
		match = re.search(r'\s*,\s*' + member + r'\("(.*)".*', line)
		if(match):
			return {"id" : match.group(1), "name" : match.group(1), "sline" : i}
		else:
			return {}


	typename = ""
	classid = ""
	procname = ""

	cr = re.compile(r"ProcessorClassIdentifier\(([A-Za-z0-9]+),\s+\"(.*)\"\)")
	nr = re.compile(r"ProcessorDisplayName\(([A-Za-z0-9]+),\s+\"(.*)\"\)")

	with fileinput.input(cppfile) as f:
		for (i,line) in enumerate(f):
			cm = cr.search(line)
			if cm:
				typename = cm.group(1)
				classid = cm.group(2)
			
			nm = nr.search(line)
			if nm:
				procname = nm.group(2)

			for member in inports.keys():
				r = get_port_init(member, i, line)
				if len(r) > 0: inports[member].update(r)

			for member in outports.keys():
				r = get_port_init(member, i, line)
				if len(r) > 0: outports[member].update(r)

			for member in properties.keys():
				r = get_property_init(member, i, line)
				if len(r) > 0: properties[member].update(r)

	mall = """/** \docpage{<CI>, <NAME>}
 * ![](<CI>.png?classIdentifier=<CI>)
 *
 * ...
 * <INPORTS>
 * <OUTPORTS>
 * <PROPERTIES>
 *
 */
"""

	if classid.find(" ") > 0:
		print_error("WARNING: space in id, " + classid, file=sys.stderr)

	inports_string = ""
	for key, val in inports.items(): inports_string += "\n *   * __" + val.get("name", key) + "__ ..."
	if len(inports_string)>0: inports_string = "\n * ### Inports" + inports_string
	 	  
	outports_string = ""
	for key, val in outports.items(): outports_string += "\n *   * __" + val.get("name", key) + "__ ..."
	if len(outports_string)>0: outports_string = "\n * ### Outports" + outports_string

	properties_string = ""
	for key, val in properties.items(): properties_string += "\n *   * __" + val.get("name", key) + "__ ..."
	if len(properties_string)>0: properties_string = "\n * ### Properties" + properties_string

	mall = mall.replace("<CI>", classid)
	mall = mall.replace("<NAME>", procname)
	mall = mall.replace("<INPORTS>", inports_string)
	mall = mall.replace("<OUTPORTS>", outports_string)
	mall = mall.replace("<PROPERTIES>", properties_string)

	return { "hfile" : hfile, "cppfile" : cppfile, "type" : typename, "id" : classid, "name" : procname,
			 "inports" : inports, "outports" : outports, "properties" : properties, "hasdoc" : has_docpage,
			 "doc" : mall}



#class IVW_MODULE_TESTING_API CompositePropertyTest : public Processor
def add_doc(file):
	d = gen(file)
	if not d["hasdoc"]:
		added = False
		bases = r"(Processor|ImageGLProcessor|DataSource<.*>|VolumeGLProcessor|CompositeProcessorGL)"
		pattern = r"^\s*class\s+\w*\s+" + d["type"] + r"\s*:\s*public\s+"+bases+"[\s,]+.*$"
		rexp = re.compile(pattern)
		with open(file, "r") as f:
			lines = f.readlines()

		with open(file, "w") as f:
			for (i,line) in enumerate(lines):
				if rexp.match(line):
					added = True
					f.write(d["doc"])
				f.write(line)
	if not added:
		print_error("Faild to add doc: " + pattern + " type: \"" + d["type"] +"\"" )		


def test(files):
	for file in files:
		d = gen(file)
		if not d["hasdoc"]:
			print_warn("Missing doc: " + file)
		else:
			print("Has doc:     "+ file)

def document(files):
	for file in files:
		d = gen(file)
		if len(d["type"]) == 0:
			print("No type     " + file)
			continue

		if not d["hasdoc"]:
			print_warn("Adding docs: " + file)
			add_doc(file)
		else:
			print("Has doc:     " + file)
