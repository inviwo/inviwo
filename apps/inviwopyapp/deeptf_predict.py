#%%
import os, sys, time, tqdm, glm
from pathlib import Path
from argparse import ArgumentParser, Namespace
import numpy as np
import torch
from torchvtk.datasets import TorchDataset
from torchvtk.utils    import TFGenerator, tex_from_pts, create_peaky_tf, random_color_generator
from skimage.io import imsave

sys.path.append('/home/dome/Repositories/inviwo-build-pystuff/bin')
import inviwopy as ivw
import inviwopyapp as qt

sys.path.append('/home/dome/Repositories/deep-tf')
from module_fourier import TFNet
from inviwopy.data       import InterpolationType
from inviwopy.properties import InvalidationLevel
from inviwopy.glm        import vec4, vec3, size2_t
import glm

from ivw_interface import InviwoInterface


# %%
if __name__=='__main__':
    parser = ArgumentParser('Volume Raycaster')
    parser.add_argument('target_path', type=str, help='Where tosave the dataset')
    parser.add_argument('--samples_per_vol', type=int, default=100)
    parser.add_argument('--resolution', type=int, default=224, help='Image resolution of the created dataset')
    parser.add_argument('--n_classes', type=int, default=15, help='Number of classes to split the TF in')
    parser.add_argument('--tf_peaks', type=int, default=1, help='Number of peaks, see train.py')
    parser.add_argument('--tf_mode', type=str, default='categorical', help='tf_mode from train.py, random, fixed, categorical, fixed_color, fixed_opacity')
    args = parser.parse_args()
    # %%
    tpath = Path(args.target_path)
    if not tpath.exists(): tpath.mkdir()
    ii = InviwoInterface('TestInject', 'volume_raycast.inv')
    vr = ii.network.VolumeRaycaster
    ii.eval()
    tf_prop = vr.properties.isotfComposite.tf
    cam = vr.properties.camera
    set_vol = ii.inject_volume_source('VolumeSource')

    ds = TorchDataset.CQ500('/run/media/dome/Data/data/torchvtk')
    for item in ds:
        print(f'Rendering {item["name"]}')
        vol = item['vol'].numpy().astype(np.float32)
        with ii.network_locked(True):
            set_vol(vol)
        for i in tqdm.trange(args.samples_per_vol):
            a = preprocess_generate_tf(item.copy(), hparams=args, tf_res=256)
            del a['vol']
            with ii.network_locked(True):
                tf_prop.clear()
                tf = a['tf_pts']
                for pt in tf:
                    tf_prop.add(pt[0], vec4(*pt[1:]))
                cam.lookFrom = vec3(get_rand_pos())

            ii.eval()
            cam.fitData()
            cam.lookFrom *= np.random.uniform(0.7, 1.1, (1,))
            ii.eval()
            im = vr.outports[0].getData().colorLayers[0].data
            im = torch.from_numpy(im).permute(2, 0, 1).contiguous()
            # print('Camera Parameters:', cam.lookFrom, cam.lookTo, cam.lookUp)
            # print('Inviwo View Mat:', cam.viewMatrix.array)
            # print('glm view mat:', glm.lookAt(cam.lookFrom, cam.lookTo, cam.lookUp))
            a['render'] = (im.float() / 255.0).half()
            a['look_from'] = torch.from_numpy(cam.lookFrom.array)
            a['look_up'] = torch.from_numpy(cam.lookUp.array)
            a['view_mat'] = torch.from_numpy(cam.viewMatrix.array)
            # a['proj_mat'] = torch.from_numpy(cam.projectionMatrix.array)
            # a['world_mat'] = torch.from_numpy(vr.inports[0].getData().worldMatrix.array)
            # a['model_mat'] = torch.from_numpy(vr.inports[0].getData().modelMatrix.array)
            a['name'] += f'_{i:04d}'
            # imsave('testuru.png', im.permute(1, 2, 0).numpy())
            # print(a['tf_rgb'], a['tf_op'])
            torch.save(a, tpath/f"{item['name']}_{i:04d}.pt")
