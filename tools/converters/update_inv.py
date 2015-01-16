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
from bs4 import BeautifulSoup

def perform(f):
    print("Open file " + f)

    with open(f, 'r') as file:
        filestr = "\n".join(file.readlines())

    soup = BeautifulSoup(filestr, 'xml')

    tfs = soup.find_all(type="TransferFunctionProperty")

    for tf in tfs:

        if "reference" in tf.attrs:
            continue

        print("BEFORE " + 60*"#")
        print(tf.prettify())
        print("AFTER  " + 60*"#")

        np = int(tf.size["content"])

        tf.size.decompose()
    
        newtf = soup.new_tag("transferFunction")

        newtf.append(soup.new_tag("maskMin", content=tf.mask_["x"]))
        newtf.append(soup.new_tag("maskMax", content=tf.mask_["y"]))

        tf.mask_.decompose()

        dps = soup.new_tag("dataPoints")
        for i in range(0,np):
            p = tf.find("pos"+str(i)).extract()
            c = tf.find("rgba"+str(i)).extract()
            p.name = "pos"
            c.name = "rgba"
        
            newp = soup.new_tag("point")
        
            newp.append(p)
            newp.append(c)
        
            dps.append(newp)

        newtf.append(dps)
        
        ipt = tf.interpolationType_.extract()
        ipt.name = "interpolationType_"
        newtf.append(ipt)

        tf.append(newtf)
        print(tf.prettify())
        print("DONE   " + 60*"#")

        
    print("Write file ")

    with open(f, 'w') as fout:
        fout.write(soup.prettify())

if __name__ == '__main__':        
    for f in sys.argv[1:]:
        perform(f)

