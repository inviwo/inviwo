import ivwpy.regression.app

from ivwpy.util import *

base = "/Users/petst/Work/Projects/Inviwo-Developent/Private"

ivwapp = base + "/builds/cmake-test/bin/Debug/inviwo.app/Contents/MacOS/inviwo"
modulepaths = [base + "/inviwo-dev/modules/", base + "/inviwo-research/modules/"]
repopaths = [base + "/inviwo-research/data/workspaces/tests/"]

out = base + "/regress"

app = ivwpy.regression.app.App(ivwapp, out, modulepaths, repopaths)
app.runTests()
