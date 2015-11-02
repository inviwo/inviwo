import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = ["C:/inviwo-dev", "C:/inviwo-research/modules"]

# Step 1:
# replace:
# 	InviwoProcessorInfo();
# with:
# 	virtual const ProcessorInfo getProcessorInfo() const override;
# 	static const ProcessorInfo processorInfo_;

n = refactoring.find_files(paths, ['*.h'], excludes=["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*"])

err = refactoring.check_file_type(n, "UTF-8")
if len(err)>0: sys.exit("Encoding errors")

n2 = refactoring.find_files(paths, ['*.cpp'], excludes=["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*"])

err2 = refactoring.check_file_type(n2, "UTF-8")
if len(err2)>0: sys.exit("Encoding errors")

pattern = r"(\s*)InviwoProcessorInfo\(\);"
replacement = r"\1virtual const ProcessorInfo getProcessorInfo() const override;\n\1static const ProcessorInfo processorInfo_;"

print("Matches:")
matches = refactoring.find_matches(n, pattern)

print("\n")
print("Replacing:")
refactoring.replace_matches(matches, pattern, replacement)


# Step 2:
# replace:
#	ProcessorDisplayName(VolumeRaycaster, "Volume Raycaster");
#	ProcessorTags(VolumeRaycaster, Tags::GL);
#	ProcessorCategory(VolumeRaycaster, "Volume Rendering");
#	ProcessorCodeState(VolumeRaycaster, CODE_STATE_STABLE);
# with:  	
# 	const ProcessorInfo VolumeRaycaster::processorInfo_{
# 	    "org.inviwo.VolumeRaycaster",  // Class identifer
# 	    "Volume Raycaster",            // Display name
# 	    "Volume Rendering",            // Category
# 	    CODE_STATE_STABLE,             // Code state
# 	    Tags::GL                       // Tags
# 	};
#  	const ProcessorInfo VolumeRaycaster::getProcessorInfo() const {
#   	return processorInfo_;
#	}

def cs(var):
	if var == "CODE_STATE_STABLE":
		return "CodeState::Stable"
	elif var == "CODE_STATE_EXPERIMENTAL":
		return "CodeState::Experimental"
	elif var == "CODE_STATE_BROKEN":
		return "CodeState::Broken"
	else:
		return var

def updatecpp(files):
	patterns = {
		"cid"   : r"""[ ]*ProcessorClassIdentifier\((\w+),\s+("[-&\.\w]+")\);?""",
		"name"  : r"""[ ]*ProcessorDisplayName\((\w+),\s*("[-& \.\w]+")\);?""",
		"tags"  : r"""[ ]*ProcessorTags\((\w+),\s+([",/:\w]+)\);?""", 
		"cat"   : r"""[ ]*ProcessorCategory\((\w+),\s+("[ \.\w]+")\);?""",
		"state" : r"""[ ]*ProcessorCodeState\((\w+),\s+([_:\w]+)\);?"""
		}

	rs = {k : re.compile(v, re.MULTILINE) for k,v in patterns.items()}

	repl = lambda x : r"""const ProcessorInfo {0:s}::processorInfo_{{
    {cid:""" +str(x)+ """s}  // Class identifier
    {name:""" +str(x)+ """s}  // Display name
    {cat:""" +str(x)+ """s}  // Category
    {state:""" +str(x)+ """s}  // Code state
    {tags:""" +str(x)+ """s}  // Tags
}};
const ProcessorInfo {0:s}::getProcessorInfo() const {{
    return processorInfo_;
}}
"""

	matching_files = []
	for file in files:
		with open(file, "r") as f:
			text = f.read()

		matches = {k : v.search(text) for k,v in rs.items()}
        	
		if all(matches.values()):
			refactoring.print_warn("Match in: " + file)
			matching_files.append(file)

			for m in matches.values():
				matched = "-->" + colorama.Fore.YELLOW + m.group(0) + colorama.Style.RESET_ALL +"<--"
				print("{0:s}".format(matched))


			cname = {k : (v.group(1) if k != "state" else cs(v.group(1))) for k,v in matches.items()}
			data = {k : (v.group(2) if k != "state" else cs(v.group(2)))+"," for k,v in matches.items()}
			slen = max([len(s) for s in data.values()])

			template = repl(slen)
			with open(file, "w") as f:
				for (i,line) in enumerate(text.split("\n")):
					ms = {k : v.search(line) for k,v in rs.items()}
	
					if ms["cid"]:
						f.write(template.format(cname["cid"], **data))
					elif any(ms.values()):
						continue
					else:
						f.write(line +"\n")


print("Updating cppfiles")
updatecpp(n2)
