import os, sys, tqdm, glm
from pathlib import Path
from argparse import ArgumentParser
import numpy as np
import torch
from torchvtk.datasets import TorchDataset

sys.path.append('/home/dome/Repositories/inviwo-build-pystuff/bin')
import inviwopy as ivw
import inviwopyapp as qt

from inviwopy.data       import InterpolationType
from inviwopy.properties import InvalidationLevel
from inviwopy.glm        import vec4, vec3

from ivw_interface import InviwoInterface

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

    peakgen =  { 'max_num_peaks': hparams.tf_peaks }
    if   hparams.tf_mode == 'random':
        peakgen['color'] = 'random'
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
        height = np.random.uniform(0.1, 0.99, (1,)).item()
        nc = hparams.n_classes
        i = np.random.randint(0, nc, 1).item()
        peaks = np.stack([np.linspace(1/20, 0.8-1/20, nc)]*2, axis=1)
        width = peaks[2, 0] - peaks[1, 0]
        peakgen['override_peaks'] = peaks[[i]]
        peakgen['height_range'] = (height, height)
        peakgen['width_range']  = (width, width)
        peakgen['colors'] = 'fixed'
        peakgen['use_hist'] = False
        peakgen['fixed_shape'] = True
        peakgen['peak_center_noise_std'] = 0
        data['tf_clas'] = torch.eye(nc)[i]
        data['tf_clas_idx'] = i
        data['tf_op'] = height
    elif hparams.tf_mode == 'categorical':
        peakgen['height_range'] = (1.0, 1.0)
        peakgen['width_range']  = (0.05, 0.05)
        peakgen['colors'] = 'fixed'
        peakgen['use_hist'] = False
        peakgen['fixed_shape'] = True
        peakgen['peak_center_noise_std'] = 0
        i = np.random.randint(0, 19, 1).item()
        peaks = np.stack([np.linspace(1/20, 1-1/20, 19)]*2, axis=1)
        peakgen['override_peaks'] = peaks[[i]]
        data['tf_clas'] = torch.eye(19)[i]
        data['tf_clas_idx'] = i
    else:
        raise Exception(f'Invalid tf_mode given: {hparams.tf_mode}')
    tfgen = TFGenerator(mode='random_peaks', peakgen_kwargs=peakgen)
    data['tf_pts'] = tfgen.generate(data['vol'], view_mat=data['view_mat'])
    data['tf_tex'] = tex_from_pts(data['tf_pts'], resolution=tf_res)

    return data


if __name__ == '__main__':
    parser = ArgumentParser('deep-tf Dataset creation')
    parser.add_argument("ds_path", type=str, help='Path to dataset from which the model makes predictions')
    parser.add_argument("target_path", type=str, help='Path to save the new dataset to')
    parser.add_argument("--samples_per_vol", type=int, default=50, help='Number of samples in the dataset to be created')
    parser.add_argument("--sample_start_idx", type=int, default=0, help='Starting index for samples_per_vol. Can be used to generate additional data, when set to previously used samples_per_vol')
    parser.add_argument('--tf_mode', type=str, help='tf_mode from train.py, random, fixed, categorical, fixed_color, fixed_opacity')
    parser.add_argument('--tf_peaks', type=int, help='Number of peaks, see train.py)
    parser.add_argument('--n_classes', type=int, default=15, help='Number of classes to split the TF in')

    args = parser.parse_args()
    tpath = Path(args.target_path)
    if not tpatch.exists(): tpath.mkdir()

    ds = TorchDataset(args.ds_path)

    # Inviwo stuff
    ii = InviwoInterface('deep-tf-ds', 'volume_raycast.inv')
    vr = ii.network.VolumeRaycaster
    tf_prop = vr.properties.isotfComposite.tf
    cam = vr.properties.camera

    set_vol = ii.inject_volume_source('VolumeSource')

    for item in tqdm(ds):
        with ii.network_locked(True):
            set_vol(item['vol'].numpy().astype(np.float32))
        for i in range(args.samples_start_idx, args.samples_per_vol+args.samples_start_idx):
            a = preprocess_generate_tf(d.copy(), hparams=args, tf_res=256)
            tf_pts = a['tf_pts'].float()
            vm     = a['view_mat'].float()

            with ii.network_locked(True):
                tf_prop.clear()
                for pt in tf_pts: tf_prop.add(pt[0], vec4(*pt[1:]))
                cam.lookFrom
                # TODO
