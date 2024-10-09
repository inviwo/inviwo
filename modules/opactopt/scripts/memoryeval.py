# Automatic performance evaluation and comparison
# approximate opacity optimisation methods. Uses
# the Direct Opacity Optimisation Renderer.
import inviwopy

from inviwopy import qt
import inviwopy.glm as glm
import ivw.utils as inviwo_utils
from inviwopy.properties import InvalidationLevel
import math
import time
import csv
import os
import sys


def sphericalToCartesian(spherical):
    r = spherical.x
    theta = spherical.y
    phi = spherical.z
    return glm.vec3(r * math.sin(theta) * math.cos(phi), r * math.sin(theta) * math.sin(phi), r * math.cos(theta))


# Global variables and initial setup
app = inviwopy.app
network = app.network

dataset = os.path.basename(network.MeshSource.filename.value)
dataset = dataset.split('-')[0].split('_')[0].split('.')[0].lower()

canvas = network.Canvas
canvas.inputSize.dimensions.value = glm.size2_t(1000, 1000)

opactopt = network.AbufferOpacityOptimisation
cam = opactopt.camera

meshInfo = network.MeshInformation # add mesh information processor to extract bounding box info
meshInfo.dataInformation.meshProperties.setChecked(True) # get bounding box info

meshMinPos = meshInfo.dataInformation.meshProperties.minPos.value
meshExtent = meshInfo.dataInformation.meshProperties.extent.value
meshCenter = meshMinPos + meshExtent / 2.0

# Initial camera settings
cam.nearPlane = 0.01
cam.farPlane = glm.length(meshExtent) * 5.0

# Smoothing settings
smoothing = opactopt.approximationProperties.smoothing
smoothing.setChecked(True)
smoothing.gaussianKernelRadius.value = 5
smoothing.gaussianKernelSigma.value = 2

camerasettings = {
    "darksky": (glm.vec3(0.22523, 1.2603, 0.3266), glm.vec3(0.0688062, 0.0688062, 0.0688062), glm.vec3(0, 1, 0), 0.1, 0.2),
    "ecmwf": (glm.vec3(275, 1.5, 1.7), glm.vec3(560, 150, 200), glm.vec3(0, 0, -1), 100, 1500),
    "heli": (glm.vec3(45, 1.28, 0.38), glm.vec3(6, 8, 10), glm.vec3(0, 1, 0), 25, 45),
    "rings": (glm.vec3(54.675, 1.174, 0.7955), glm.vec3(6.36512, 8.4615, 9.9624), glm.vec3(-0.7, 0.6731, 0.1904), 25, 50),
    "tornado": (glm.vec3(40, 1.68, -1.66), glm.vec3(0.25, 0, 0), glm.vec3(0, 0, 1), 30, 50)
}

nframes = 1000
nrotations = 5
nvertical = 2
def rotateCamera(frame):
    # spiral in
    network.lock()
    cam.lookFrom = meshCenter + glm.vec3(5.0 - 4.7 * (frame/nframes), 5.0 - 4.7 * (frame/nframes), 0.8) * \
            glm.vec3(math.cos(2 * math.pi * nrotations * (frame / nframes)), math.sin(2 * math.pi * nrotations * (frame / nframes)), math.sin(2 * math.pi * nvertical * (frame / nframes))) * \
                     meshExtent
    cam.lookTo = glm.vec3(meshCenter.x, meshCenter.y, cam.lookFrom.z + 0.5 * (meshCenter.z - cam.lookFrom.z))
    cam.lookUp = glm.vec3(0, 0, 1)
    network.unlock()


savepath = "C:/Inviwo/results/"
filename = "memoryeval.csv"
fields = ["Dataset", "Initial", "Min", "Max"]
fragsToMB = 4 * 4 * 1e-6
initialfrags = 0
minfrags = math.inf
maxfrags = 0

# Approximation settings
ap = opactopt.approximationProperties
ap.approximationMethod.selectedIndex = 0
print(ap.approximationMethod.selectedDisplayName)

ncoeffs = 9
ap.importanceSumCoefficients.value = ncoeffs
ap.opticalDepthCoefficients.value = ncoeffs

# set initial camera parameters
cam.setLook(sphericalToCartesian(camerasettings[dataset][0]), camerasettings[dataset][1], camerasettings[dataset][2])
inviwo_utils.update()
initialfrags = opactopt.nfrags.value
minfrags = min(minfrags, opactopt.nfrags.value)
maxfrags = max(maxfrags, opactopt.nfrags.value)
    
for i in range(nframes):             
    rotateCamera(i)
    inviwo_utils.update()
    minfrags = min(minfrags, opactopt.nfrags.value)
    maxfrags = max(maxfrags, opactopt.nfrags.value)
      
with open(savepath + filename, 'a', newline='') as f:
    writer = csv.writer(f, delimiter=',')
    # writer.writerow(fields)
    writer.writerow([dataset, initialfrags, minfrags, maxfrags])

print("Done")
