#requires lxml, and beautifulsoup4

#To install lxml
#Install lxml on Windows: https://pypi.python.org/pypi/lxml/3.3.3

#To install beautifulsoup4
#In Windows, run cmd as admin"
#Run "python get-pip.py" script to install pip
#Navigate to "PythonPath/Scripts"
#Run "pip install beautifulsoup4"

import sys
import os
import re
from bs4 import BeautifulSoup

# custom indentation for bs4.BeatuifulSoup.prettify
# https://stackoverflow.com/questions/15509397/custom-indent-width-for-beautifulsoup-prettify
orig_prettify = BeautifulSoup.prettify
r = re.compile(r'^(\s*)', re.MULTILINE)
def prettify(self, encoding=None, formatter="minimal", indent_width=4):
    return r.sub(r'\1' * indent_width, orig_prettify(self, encoding, formatter))
BeautifulSoup.prettify = prettify

def perform(f):
    print("Open file " + f)

    with open(f, 'r') as file:
        filestr = "\n".join(file.readlines())

    soup = BeautifulSoup(filestr, 'xml')
	
    treedata = soup.find_all("InviwoTreeData")

    for tf in treedata:
		
        if "reference" in tf.attrs:
            continue

        print("BEFORE " + 60*"#")
        print(tf.prettify())
        print("AFTER  " + 60*"#")
    
        newtf = soup.new_tag("InviwoWorkspace", version="2")

        newtf.append(tf.maskMin)
        newtf.append(tf.maskMax)

        dps = soup.new_tag("Points")
        print(tf.point)

        for i in tf.find_all("point"):
            newp = soup.new_tag("Point")
        
            newp.append(soup.new_tag("pos", content=i.pos["x"]))
            newp.append(i.rgba)
        
            dps.append(newp)

        newtf.append(dps)

        tf.decompose()
		
        soup.append(newtf)
        print(newtf.prettify())
        print("DONE   " + 60*"#")

        
    print("Write file ")

    with open(f, 'w') as fout:
        fout.write(soup.prettify())

if __name__ == '__main__':        
    for f in sys.argv[1:]:
        perform(f)

