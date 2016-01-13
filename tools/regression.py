import ivwpy.regression.app
import ivwpy.regression.inviwoapp

from ivwpy.util import *

# Ipython auto reaload
# %load_ext autoreload
# %autoreload 2

#base = "/Users/petst/Work/Projects/Inviwo-Developent/Private"
#ivwapp = base + "/builds/cmake-test/bin/Debug/inviwo.app/Contents/MacOS/inviwo"

base = "C:/Users/petst55/Work/Inviwo"
ivwapp = base + "/Builds/build-git-VS2015-64bit-Qt5.5.0/bin/Debug/inviwo-cli.exe"

modulepaths = [base + "/inviwo-dev/modules", base + "/inviwo-research/modules"]
repopaths = [base + "/inviwo-research/data/workspaces/tests/"]

out = base + "/regress"

app = ivwpy.regression.app.App(ivwapp, out, modulepaths, repopaths, 
	settings=ivwpy.regression.inviwoapp.RunSettings(timeout=60))

app.runTests(testrange = slice(0,None))

app.saveJson(out+"/report.json")
app.saveHtml(out+"/report.html")
