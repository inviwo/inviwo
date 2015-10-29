import sys 
import os
import re
import colorama
colorama.init()

sys.path.append(os.path.abspath(r"C:\Users\petst55\Work\Inviwo\Inviwo-dev\tools"))
import refactoring

paths = ["C:/Users/petst55/Work/Inviwo/Inviwo-dev", "C:/Users/petst55/Work/Inviwo/Inviwo-research"]

excludespatterns = ["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*", "*/vrnbase/*"];

files = refactoring.find_files(paths, ['*.h', '*.cpp'], excludes=excludespatterns)

matches = refactoring.find_matches(files, r":\s*public\s+Processor")

processorswithinit = set(refactoring.find_matches(matches, r"initialize\(\)"))

processorswithdeinit = set(refactoring.find_matches(matches, r"deinitialize\(\)"))

lastpart = lambda x: (x.split(r"/Inviwo/")[-1]).split("\\modules\\")[-1]

print("")
print("")
print("## Prcessor with initialize/deinitialize")

l1 = list(processorswithinit.intersection(processorswithdeinit))
l1.sort()

l2 = list(processorswithinit.difference(processorswithdeinit))
l2.sort()

l3 = list(processorswithdeinit.difference(processorswithinit))
l3.sort()

list(map(lambda x: print("* [ ] " + lastpart(x)), l1))
print("")
print("## Prcessor with initialize")
list(map(lambda x: print("* [ ] " + lastpart(x)), l2))
print("")
print("## Prcessor with deinitialize")
list(map(lambda x: print("* [ ] " + lastpart(x)), l3))