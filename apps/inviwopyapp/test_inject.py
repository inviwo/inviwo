#%%
import os, sys
from pathlib import Path
import numpy as np


sys.path.append('/home/dome/Repositories/inviwo-build/bin')
import inviwopy as ivw
import inviwopyapp as qt

from inviwopy.data       import InterpolationType
from inviwopy.properties import InvalidationLevel

from ivw_interface import InviwoInterface
from volume_injector import VolumeInjector
#%%
if __name__=='__main__':
    # %%
    ii = InviwoInterface('TestInject', 'boron.inv')
    set_vol = ii.inject_volume_source('VolumeSource')
    # ii.app.run()
    ii.network.lock()
    set_vol(np.random.rand(200,200,200))
    ii.network.VolumeRaycaster.properties.camera.fitData()
    ii.network.VolumeRaycaster.invalidate(InvalidationLevel.InvalidOutput)
    ii.network.unlock()
    # ii.app.waitForNetwork()
    print('Injector hasData?', ii.network.VolumeInjector.outports[0].hasData())
    ii.network.save(ii.app.getPath(ivw.PathType.Workspaces) + '/injected.inv')

    ii.app.waitForNetwork(1)
    print('Snapshotting')
    print('Canvas inport hasData?', ii.network.Canvas.inports[0].getData().colorLayers[0])
    im = ii.network.Canvas.inports[0].getData().colorLayers[0]
    im.interpolation = InterpolationType.Linear
    ar = im.data
    print(type(ar), ar.shape, np.mean(ar > 0))
    # im.save('ivwim_save.png')
    ii.network.Canvas.snapshot('test.png')




# %%
