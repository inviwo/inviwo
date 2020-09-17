#%%
import os, sys, time
from pathlib import Path
import numpy as np


sys.path.append('/home/dome/Repositories/inviwo-build-pystuff/bin')
import inviwopy as ivw
import inviwopyapp as qt

from inviwopy.data       import InterpolationType
from inviwopy.properties import InvalidationLevel

from ivw_interface   import InviwoInterface
from volume_injector import VolumeInjector
#%%
if __name__=='__main__':
    # %%
    ii = InviwoInterface('TestInject', 'boron.inv')
    set_vol = ii.inject_volume_source('VolumeSource')

    set_vol(np.random.rand(200,200,200))
    ii.network.VolumeRaycaster.properties.camera.fitData()
    ii.app.waitForPool()
    ii.network.Canvas.snapshot('test2.png')





# %%
