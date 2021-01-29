#%%
import os, sys, time, tqdm
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

from inviwopy.data       import InterpolationType
from inviwopy.properties import InvalidationLevel
from inviwopy.glm        import vec4, vec3, size2_t
import glm

from ivw_interface import InviwoInterface
#%%
def get_rand_pos(length=1):
    pos = np.random.normal(0, 1, (3,))
    return length * pos / np.linalg.norm(pos)

def preprocess_generate_tf(data, hparams, dtype=None, tf_res=512):
    # Generate random TF based on volume intensity
    data['vol'] = data['vol'].to(dtype)
    # tf_pts = random_tf_from_vol(data['vol'])
    # data['tf_tex'] = tex_from_pts(tf_pts, resolution=tf_res)

    # Generate random view point, camera matrix (like inviwo does)
    look_from = np.random.normal(0, 1, (3,)) # rand pos with distance in [2,3]
    look_from = glm.vec3(look_from / np.linalg.norm(look_from) * np.random.uniform(2, 3, (1,)))
    look_to = glm.vec3(0.5)
    up = glm.vec3(0.0, 1, 0)
    view_dir = glm.normalize(look_to - look_from)
    right = glm.cross(view_dir, up)
    look_up = glm.cross(right, view_dir)
    data['view_mat'] = torch.tensor(glm.lookAt(look_from, look_to, look_up).to_list())

    peakgen =  { 'max_num_peaks': hparams.tf_peaks, 'use_hist': False }
    if   hparams.tf_mode == 'random':
        peakgen['colors'] = 'random'
    elif hparams.tf_mode == 'fixed':
        peakgen['height_range'] = (1.0, 1.0)
        peakgen['width_range']  = (0.1, 0.1)
        peakgen['colors'] = 'fixed'
        peakgen['use_hist'] = False
        peakgen['fixed_shape'] = True
    elif hparams.tf_mode == 'fixed_color':
        peakgen['colors'] = 'fixed'
        peakgen['height_range'] = (0.3, 1.0)
        peakgen['width_range']  = (0.01, 0.1)
        peakgen['use_hist'] = False
    elif hparams.tf_mode == 'fixed_opacity':
        peakgen['color'] = 'random'
        peakgen['height_range'] = (1.0, 1.0)
        peakgen['widht_range']  = (0.1, 0.1)
        peakgen['use_hist'] = False
        peakgen['fixed_shape'] = True
    elif hparams.tf_mode == 'categorical_randop':
        height = np.random.uniform(0.1, 0.99, (hparams.tf_peaks,))
        nc = hparams.n_classes
        i = np.random.choice(list(range(nc)), (hparams.tf_peaks,), replace=False)
        op = np.random.uniform(0.1, 0.99, (hparams.tf_peaks,))
        peaks = np.stack([np.linspace(1/20, 0.8-1/20, nc)]*2, axis=1)
        width = (peaks[2, 0] - peaks[1, 0]) / 2.0
        peaks = peaks[i]
        widths = [width] * hparams.tf_peaks
        peaks[:, 1] = height
        data['tf_pts'] = create_peaky_tf(peaks, widths)
        data['tf_tex'] = tex_from_pts(data['tf_pts'], resolution=tf_res)
        data['tf_clas'] = torch.zeros(nc)
        data['tf_clas'][i] = 1
        data['tf_clas_idx'] = i
        data['tf_op'] = height
        return data
    elif hparams.tf_mode == 'categorical_rand':
        height = np.random.uniform(0.1, 0.99, (hparams.tf_peaks,))
        color_gen = random_color_generator()
        colors = np.stack([next(color_gen) for i in range(hparams.tf_peaks)])
        nc = hparams.n_classes
        i = np.random.choice(list(range(nc)), (hparams.tf_peaks,), replace=False)
        op = np.random.uniform(0.1, 0.99, (hparams.tf_peaks,))
        peaks = np.stack([np.linspace(1/20, 0.8-1/20, nc)]*2, axis=1)
        width = (peaks[2, 0] - peaks[1, 0]) / 2.0
        peaks = peaks[i]
        widths = [width] * hparams.tf_peaks
        peaks[:, 1] = height
        peaks = np.concatenate([peaks, colors], axis=1)
        data['tf_pts'] = create_peaky_tf(peaks, widths)
        data['tf_tex'] = tex_from_pts(data['tf_pts'], resolution=tf_res)
        data['tf_clas'] = torch.zeros(nc)
        data['tf_clas'][i] = 1
        data['tf_clas_idx'] = i
        data['tf_op'] = height
        data['tf_rgb'] = colors
        return data
    elif hparams.tf_mode == 'categorical':
        nc = hparams.n_classes
        i = np.random.choice(list(range(nc)), (hparams.tf_peaks,), replace=False)
        peaks = np.stack([np.linspace(1/20, 0.8-1/20, nc)]*2, axis=1)
        width = (peaks[2, 0] - peaks[1, 0]) / 2.0
        peakgen['override_peaks'] = peaks[i]
        peakgen['height_range'] = (0.99, 0.99)
        peakgen['width_range']  = (width, width)
        peakgen['colors'] = 'fixed'
        peakgen['use_hist'] = False
        peakgen['fixed_shape'] = True
        peakgen['peak_center_noise_std'] = 0
        peakgen['max_num_peaks'] = None # Use len(peaks)
        data['tf_clas'] = torch.zeros(nc)
        data['tf_clas'][i] = 1
        data['tf_clas_idx'] = i
    else:
        raise Exception(f'Invalid tf_mode given: {hparams.tf_mode}')
    tfgen = TFGenerator(mode='random_peaks', peakgen_kwargs=peakgen)
    data['tf_pts'] = tfgen.generate(data['vol'], view_mat=data['view_mat'])
    data['tf_tex'] = tex_from_pts(data['tf_pts'], resolution=tf_res)

    return data

# %%
if __name__=='__main__':
    parser = ArgumentParser('Volume Raycaster')
    parser.add_argument('target_path', type=str, help='Where tosave the dataset')
    parser.add_argument('--samples_per_vol', type=int, default=100)
    parser.add_argument('--fixed_pos', action='store_true', help='Use same camera pos for all renders')
    parser.add_argument('--resolution', type=int, default=224, help='Image resolution of the created dataset')
    parser.add_argument('--n_classes', type=int, default=15, help='Number of classes to split the TF in')
    parser.add_argument('--tf_peaks', type=int, default=1, help='Number of peaks, see train.py')
    parser.add_argument('--tf_mode', type=str, default='categorical', help='tf_mode from train.py, random, fixed, categorical, fixed_color, fixed_opacity')
    parser.add_argument('--start_idx', type=int, default=0, help='Starting index to generate additional data. Use old samples_per_vol here')
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
    for j, item in enumerate(ds):
        if j > 0: break
        print(f'Rendering {item["name"]}')
        vol = item['vol'].numpy().astype(np.float32)
        with ii.network_locked(True):
            set_vol(vol)
        for i in tqdm.trange(args.start_idx, args.start_idx+args.samples_per_vol):
            a = preprocess_generate_tf(item.copy(), hparams=args, tf_res=256)
            del a['vol']
            with ii.network_locked(True):
                tf_prop.clear()
                tf = a['tf_pts']
                for pt in tf:
                    tf_prop.add(pt[0], vec4(*pt[1:]))
                # cam.lookFrom = vec3(0.0,3.0, 0.0)
                if args.fixed_pos:
                    cam.lookFrom = vec3(-0.12641, -1.959986, 0.4977)
                else:
                    cam.lookFrom = vec3(get_rand_pos())
                cam.lookUp = vec3(0,-1.0,0)
                # print(cam.lookFrom)

            ii.eval()
            cam.fitData()
            # cam.lookFrom *= np.random.uniform(0.7, 1.1, (1,))
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
            torch.save(a, tpath/f"{item['name']}_{i:06d}.pt")
