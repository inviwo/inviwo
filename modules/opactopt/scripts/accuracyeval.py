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
from PIL import Image, ImageDraw


def sphericalToCartesian(spherical):
    r = spherical.x
    theta = spherical.y
    phi = spherical.z
    return glm.vec3(r * math.sin(theta) * math.cos(phi), r * math.sin(theta) * math.sin(phi), r * math.cos(theta))


# Global variables and initial setup
app = inviwopy.app
network = app.network

dataset = str(os.path.basename(network.MeshSource.filename.value))

canvas = network.Canvas
canvas.inputSize.dimensions.value = glm.size2_t(1000, 1000)

opactopt = None
optimiser = "None"
try:
    opactopt = network.AbufferOpacityOptimisation
    optimiser = "A-buffer"
except:
    pass

try:
    opactopt = network.DecoupledOpacityOptimisation
    optimiser = "Decoupled"
except:
    pass

if opactopt is None:
    sys.exit()
    
cam = opactopt.camera

meshInfo = network.MeshInformation # add mesh information processor to extract bounding box info
meshInfo.dataInformation.meshProperties.setChecked(True) # get bounding box info

meshMinPos = meshInfo.dataInformation.meshProperties.minPos.value
meshExtent = meshInfo.dataInformation.meshProperties.extent.value
meshCenter = meshMinPos + meshExtent / 2.0



# Coefficient amount for each approximation method
coefficients = {
    "fourier": [5, 7, 9, 11, 13, 15, 17, 19, 21],
    "legendre": [5, 7, 9, 11, 13, 15],
    "piecewise": [5, 7, 9, 11, 13, 15, 17, 19, 21],
    "powermoments": [5, 7, 9],
    "trigmoments": [5, 7, 9]
}

evalpixels = {
    "darksky": [(828, 650)], # (227, 184)
    "ecmwf": [(818, 849)], # (414, 810)
    "heli": [(337, 551),], # (712, 882)
    "rings": [(411, 499)], # (614, 408)
    "tornado": [(556, 686)], # (706, 516)
}

camerasettings = {
    "darksky": (glm.vec3(0.22523, 1.2603, 0.3266), glm.vec3(0.0688062, 0.0688062, 0.0688062), glm.vec3(0, 1, 0), 0.1, 0.2),
    "ecmwf": (glm.vec3(275, 1.5, 1.7), glm.vec3(560, 150, 200), glm.vec3(0, 0, -1), 100, 1500),
    "heli": (glm.vec3(45, 1.28, 0.38), glm.vec3(6, 8, 10), glm.vec3(0, 1, 0), 25, 45),
    "rings": (glm.vec3(54.675, 1.174, 0.7955), glm.vec3(6.36512, 8.4615, 9.9624), glm.vec3(-0.7, 0.6731, 0.1904), 25, 50),
    "tornado": (glm.vec3(40, 1.68, -1.66), glm.vec3(0.25, 0, 0), glm.vec3(0, 0, 1), 30, 50)
}

dataset = dataset.split('-')[0].split('_')[0].split('.')[0].lower()
savepath = "C:/Inviwo/results/accuracy/"

# set camera parameters
cam.setLook(sphericalToCartesian(camerasettings[dataset][0]), camerasettings[dataset][1], camerasettings[dataset][2])
cam.nearPlane = camerasettings[dataset][3]
cam.farPlane = camerasettings[dataset][4]


if optimiser == "A-buffer":
    
    # create debug locations image
    ap = opactopt.approximationProperties
    am = ap.approximationMethod

    # Rendering settings
    opactopt.approximationProperties.smoothing.setChecked(True)
    opactopt.approximationProperties.smoothing.gaussianKernelRadius.value = 20
    opactopt.approximationProperties.smoothing.gaussianKernelSigma.value = 5
    ap.useExactBlending.value = True
    am.selectedIndex = 0
    ap.importanceSumCoefficients.value = coefficients['fourier'][-1]
    ap.opticalDepthCoefficients.value = coefficients['fourier'][-1]

    opactopt.invalidationLevel = InvalidationLevel.InvalidResources
    inviwo_utils.update()

    evalloc_image = savepath + 'img/evalloc/' + dataset + '-evalloc.png'
    if os.path.exists(evalloc_image):
        os.remove(evalloc_image)
    canvas.snapshot(evalloc_image)

    with Image.open(evalloc_image).convert('RGBA') as img:
        draw = ImageDraw.Draw(img)

        for pixel in evalpixels[dataset]:
            draw.rectangle([(pixel[0] - 50, canvas.inputSize.dimensions.value.y - (pixel[1] + 50)),
                            (pixel[0] + 50, canvas.inputSize.dimensions.value.y - (pixel[1] - 50))], outline='greenyellow', width=20)

        img.save(evalloc_image)

    # gather approximation accuracy results
    # opactopt.approximationProperties.smoothing.setChecked(False)
    # inviwo_utils.update()
    
    # for approx_idx in range(am.size):
    #     am.selectedIndex = approx_idx
    #     approxname = am.selectedDisplayName
    #     print(approxname)
    #     print(coefficients[am.selectedIdentifier])

        # for ncoeffs in coefficients[am.selectedIdentifier]:
        #     ap.importanceSumCoefficients.value = ncoeffs
        #     ap.opticalDepthCoefficients.value = ncoeffs

            # Use approximation debug
            # for pixel in evalpixels[dataset]:
            #     print("Debug pixel: " + str(pixel))
            #     filename = "data/{}/{}-{}-{}-{}-approx.txt".format(dataset, am.selectedIdentifier, pixel[0], pixel[1], ncoeffs)
            #     ap.debugFile.value = savepath + filename
            #     ap.debugCoords.value = glm.ivec2(pixel[0], pixel[1])
            #     ap.useExactBlending.value = False
            #     inviwo_utils.update()
            #     ap.debugApproximation.press()


            # extract images for image comparison
            # for exactblending in [False, True]:
            #     ap.useExactBlending.value = exactblending
            #     inviwo_utils.update()

            #     approximage = savepath + "img/approx/{}/{}-{}{}.png".format(dataset, am.selectedIdentifier, ncoeffs, "-exactblending" if exactblending else "")
            #     if os.path.exists(approximage):
            #         os.remove(approximage)
            #     canvas.snapshot(approximage)
elif optimiser == "Decoupled":
    image = savepath + "img/decoupled/{}.png".format(dataset)
    if os.path.exists(image):
        os.remove(image)
    inviwo_utils.update()
    canvas.snapshot(image)
        

print("Done")
