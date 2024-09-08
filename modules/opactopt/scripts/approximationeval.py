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


# Global variables and initial setup
app = inviwopy.app
network = app.network

dataset = os.path.basename(network.MeshSource.filename.value)

canvas = network.Canvas
canvas.inputSize.dimensions.value = glm.size2_t(1000, 1000)

opactopt = network.DirectOpacityOptimisation
cam = opactopt.camera

meshInfo = network.MeshInformation # add mesh information processor to extract bounding box info
meshInfo.dataInformation.meshProperties.setChecked(True) # get bounding box info

meshMinPos = meshInfo.dataInformation.meshProperties.minPos.value
meshExtent = meshInfo.dataInformation.meshProperties.extent.value
meshCenter = meshMinPos + meshExtent / 2.0

# Initial camera settings
cam.nearPlane = glm.length(meshExtent) * 0.5
cam.farPlane = glm.length(meshExtent) * 2.0

# Smoothing settings
smoothing = opactopt.approximationProperties.smoothing
smoothing.setChecked(True)
smoothing.gaussianKernelRadius.value = 5
smoothing.gaussianKernelSigma.value = 2

nframes = 10000
timers = [opactopt.timeSetup, opactopt.timeProjection, opactopt.timeSmoothing,
          opactopt.timeImportanceApprox, opactopt.timeBlending, opactopt.timeNormalisation]
timevals = [0.0] * (2 + len(timers))

# Coefficient amount for each approximation method
coefficients = {
    "fourier": [5, 7, 9, 15],
    "legendre": [5, 7, 9, 11],
    "piecewise": [5, 7, 9, 15],
    "powermoments": [5, 7, 9],
    "trigmoments": [5, 7, 9]
}

nrotations = 20
def rotateCamera(frame):
    # update camera
    network.lock()
    if (i < (nframes / 2)):
        # rotate around z-axis
        axis = glm.vec3(0, 0, 1)
        angle = 2 * math.pi * (nrotations * frame / (nframes / 2))
        cam.lookUp = glm.vec3(0,0,1)
    else:
        # rotate around y-axis
        axis = glm.vec3(0, 1, 0)
        angle = 2 * math.pi * (nrotations * frame / (nframes / 2) - 1)        
        cam.lookUp = glm.rotate(glm.vec3(0,0,1), angle, axis)

    cam.lookFrom = meshCenter + glm.rotate(glm.vec3(1,0,0), angle, axis) * 1.5 * glm.length(meshExtent)
    network.unlock()


def updateTimers(appframetime):
    # if opactopt.timingMode.value == 1:
    #     timevals[1] += opactopt.timeTotal.value / 1e6
    # el
    if opactopt.timingMode.value == 2:
        timevals[0] = appframetime
        for i in range(len(timers)):
            time_ms = timers[i].value / 1e6
            timevals[1] += time_ms #add to total time
            timevals[i + 2] = time_ms


savepath = "C:/Inviwo/results/"
filename = dataset + "-approximationeval.csv"
fields = ["Approximation method", "Coefficients", "App time", "Sum time", "Setup time", "Projection time", "Smoothing time", "Importance approx time", "Blending time", "Normalisation time"]
opactopt.timingMode.value = 2
network.lock()
with open(savepath + filename, 'w', newline='') as f:
    print("Writing to: " + savepath + filename)
    # File header
    writer = csv.writer(f, delimiter=',')
    writer.writerow(fields)

    for approx_idx in range(opactopt.approximationProperties.approximationMethod.size):
        ap = opactopt.approximationProperties
        ap.approximationMethod.selectedIndex = approx_idx
        print(ap.approximationMethod.selectedDisplayName)

        for ncoeffs in coefficients[ap.approximationMethod.selectedIdentifier]:
            print(ncoeffs)

            ap.importanceSumCoefficients.value = ncoeffs
            ap.opticalDepthCoefficients.value = ncoeffs

            # warm up after change n coefficients/rendering method
            network.unlock()
            for _ in range(5):
                opactopt.invalidate(InvalidationLevel.InvalidOutput)
                inviwo_utils.update()
    
            for i in range(nframes):
                timevals = [0.0] * len(timevals)
                start = time.time()
                
                rotateCamera(i)
                inviwo_utils.update()

                updateTimers((time.time() - start)*1e3)
                writer.writerow([ap.approximationMethod.selectedDisplayName, ncoeffs] + timevals)

print("Done")
opactopt.timingMode.value = 0
