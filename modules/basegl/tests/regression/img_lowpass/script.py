# Inviwo Python script
import inviwopy
import inviwopy.qt
import time
import ivw.regression

m = ivw.regression.Measurements()

imageLowPass = inviwopy.app.network.ImageLowPass
canvas = inviwopy.app.network.Lowpass

for g in range(0, 2):
    isGauss = g == 0
    imageLowPass.gaussian.value = isGauss
    name = "Gaussian-" if isGauss else "Box-"

    r = [1, 3, 5, 10, 15, 30, 32] if isGauss else [1, 2, 3, 5, 11, 17, 32, 60]
    p = imageLowPass.sigma if isGauss else imageLowPass.kernelSize

    for size in r:
        start = time.perf_counter()
        p.value = size
        inviwopy.qt.update()
        ivw.regression.saveCanvas(canvas, name + str(size) + "x" + str(size))
        end = time.perf_counter()
        m.addFrequency(name + 'time-' + str(size) + "x" + str(size), end - start)

m.save()
