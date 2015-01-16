#Run "python update-all-itf-in-folder.py ABSOLUTE_FOLDER_PATH

import sys
import os
import update_itf

absolute_path = sys.argv[1]
print "Searching in: " + absolute_path

for root, dirs, files in os.walk(absolute_path):
    for file in files:
        if file.endswith(".itf"):
             print "Converting: " + file
             update_itf.perform(os.path.join(root, file))


