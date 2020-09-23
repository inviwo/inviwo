#%%
import os, sys, time, tqdm
from pathlib import Path
from argparse import ArgumentParser
import numpy as np
import torch
from torchvtk.datasets import TorchDataset
from torchvtk.utils    import random_tf_from_vol
from skimage.io import imsave

sys.path.append('/home/dome/Repositories/inviwo-build-pystuff/bin')
import inviwopy as ivw
import inviwopyapp as qt

from inviwopy.data       import InterpolationType
from inviwopy.properties import InvalidationLevel
from inviwopy.glm        import vec4, vec3

from ivw_interface   import InviwoInterface
from volume_injector import VolumeInjector
#%%
def get_rand_pos(length=1):
    pos = np.random.normal(0, 1, (3,))
    return length * pos / np.linalg.norm(pos)

if __name__=='__main__':
    parser = ArgumentParser('Volume Raycaster')
    parser.add_argument('-samples-per-volume', type=int, default=100)
    args = parser.parse_args()
    # %%
    ii = InviwoInterface('TestInject', 'volume_raycast.inv')
    vr = ii.network.VolumeRaycaster
    tf_prop = vr.properties.isotfComposite.tf
    cam = vr.properties.camera
    set_vol = ii.inject_volume_source('VolumeSource')

    ds = TorchDataset.CQ500('/run/media/dome/Data/data/torchvtk')
    ds.items = ds.items
    for item in ds:
        print(f'Rendering {item["name"]}')
        vol = item['vol'].numpy().astype(np.float32)
        with ii.network_locked(True):
            set_vol(vol)
        for i in tqdm.trange(args.samples_per_volume):
            with ii.network_locked(True):
                tf_prop.clear()
                tf = random_tf_from_vol(vol)
                for pt in tf:
                    tf_prop.add(pt[0], vec4(*pt[1:]))
                cam.lookFrom = vec3(get_rand_pos())

                ii.eval()
                cam.fitData()
                cam.lookFrom *= np.random.uniform(0.7, 1.1, (1,))

            im = vr.outports[0].getData().colorLayers[0].data
            im = torch.from_numpy(im).permute(2, 1, 0).contiguous()
            data = {
                'render': im,
                'look_from': torch.from_numpy(cam.lookFrom.array),
                'look_up': torch.from_numpy(cam.lookUp.array),
                'tf': tf,
                'vol_name': item['name']
            }
            torch.save(data, f"/run/media/dome/Data/data/deep-tf/{item['name']}_{i:04d}.pt")
