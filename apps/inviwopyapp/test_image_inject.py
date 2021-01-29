# %%
import sys
sys.path.append('/home/dome/Repositories/inviwo-build-pystuff/bin')
import inviwopy as ivw
import inviwopyapp as qt
import numpy as np

from ivw_interface import InviwoInterface
sys.path.append('/home/dome/Repositories/inviwo-pystuff/modules/python3/processors')
from ImageInjector import ImageInjector
import inviwopy as ivw

# Init InviwoInterface
ii = InviwoInterface('TestImageInject', 'show_image.inv')
canvas = ii.network.Canvas
# Replace processor, get callback
setArray = ii.inject_image_source('Image Source')
# Set data, (also invalidates)
setArray(np.random.uniform(0,1, (255,255,3)).astype(np.float32))
# Wait for evaluation
ii.eval()
# Take snapshot. Should show the above random array
canvas.snapshot('iminjtest.png')
