#Run "python update-all-inv-in-folder.py ABSOLUTE_FOLDER_PATH

import sys
import os
import update_inv

absolute_path = sys.argv[1]
print "Searching in: " + absolute_path

for root, dirs, files in os.walk(absolute_path):
    for file in files:
        if file.endswith(".inv"):
             print "Converting: " + file
             update_inv.perform(os.path.join(root, file))


