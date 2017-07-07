
import numpy as np
import scipy.misc


def save(img,path):
    print(path)

    scipy.misc.toimage(np.rot90(img), cmin=0.0, cmax=...).save(path)
