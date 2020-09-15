import os, sys
from pathlib import Path
import numpy as np

sys.path.append('/home/dome/Repositories/inviwo-build/bin/')
import inviwopy as ivw
from inviwopy.glm import vec4, dvec2, size2_t
from inviwopy.properties import InvalidationLevel
import inviwopyapp as qt


#%%
if __name__ == '__main__':
    #%%
    lc = ivw.LogCentral()
    cl = ivw.ConsoleLogger()
    lc.registerLogger(cl)

    # Create the inviwo application
    app = qt.InviwoApplicationQt()
    app.registerModules()

    # load a workspace
    app.network.load(app.getPath(ivw.PathType.Workspaces) + "/volume_rendering.inv")

    src = app.network.VolumeInjector
    vr = app.network.VolumeRaycaster
    canvas = app.network.Canvas

    vol = np.random.rand(200,200,200).astype(np.float32)

    app.network.lock()

    src.array = vol
    src.invalidate(InvalidationLevel.InvalidOutput)
    vr.properties.camera.fitData()

    app.network.unlock()
    app.waitForNetwork()

    print('done.')
    print('VolumeInjector outport: ', src.outports[0].getData().data.shape)
    print('VR volume inport: ', vr.inports[0].getData().data.shape)

    im = vr.outports[0].getData().colorLayers[0].data
    print('VR outport image: ', im.shape, im.dtype)
    if im.max() == 0.0: print('Raycaster has no output!')
    else:
        canvas.snapshot('snapshot.png')
        app.waitForNetwork()
    # run the app event loop
