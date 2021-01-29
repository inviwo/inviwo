#%%
import os, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

sys.path.append('/home/dome/Repositories/inviwo-build/bin/')
import inviwopy as ivw
from inviwopy.glm import vec4, dvec2, size2_t
from inviwopy.properties import InvalidationLevel
import inviwopyapp as qt

from torchvtk.utils import random_tf_from_vol, normalize_hounsfield

#%%
if __name__ == '__main__':
    #%%
    lc = ivw.LogCentral()
    cl = ivw.ConsoleLogger()
    lc.registerLogger(cl)

    # Create the inviwo application
    app = qt.InviwoApplicationQt("VolumeRaycasting")
    app.registerModules()
    app.resizePool(0)

    # load a workspace
    app.network.load(app.getPath(ivw.PathType.Workspaces) + "/volume_rendering.inv")


    src = app.network.VolumeInjector
    vr = app.network.VolumeRaycaster
    canvas = app.network.Canvas

    # %%
    # Make sure the app is ready
    # app.waitForNetwork()

    path = Path('/run/media/dome/Data/data/Volumes/QureNpy')
    its = [path/nam for nam in os.listdir(path)]

    vol = normalize_hounsfield(np.load(its[2]))
    app.network.lock()
    src.array = vol
    src.invalidate(InvalidationLevel.InvalidOutput)
    vr.properties.camera.fitData()
    tf_prop = vr.properties.isotfComposite.tf
    tf_pts = random_tf_from_vol(vol)
    tf_prop.clear()
    for c, r, g, b, o in tf_pts:
        tf_prop.add(c, vec4(r,g,b,o))
    vr.invalidate(InvalidationLevel.InvalidOutput)
    canvas.size = size2_t(512, 512)
    app.network.unlock()
    app.waitForNetwork()
    print('done.')
    print('VolumeInjector outport: ', src.outports[0].getData().data.shape)
    print('VR volume inport: ', vr.inports[0].getData().data.shape)

    im = vr.outports[0].getData().colorLayers[0].data
    print('VR outport image: ', im.shape, im.dtype)
    if im.max() == 0.0:
        print('Raycaster has no output!')
    else:
        print('Saving!')
        Image.fromarray(im).save('save.png')
        canvas.snapshot('snapshot.png')
        app.waitForNetwork()
    # run the app event loop
    # app.run()


# %%
