# Inviwo Python script
import inviwopy
from inviwopy.glm import vec3
import ivw.utils as inviwo_utils
import math
import time

# Use for example the boron example network

app = inviwopy.app
network = app.network

cam = network.EntryExitPoints.camera

old_lookto = cam.lookTo
old_lookup = cam.lookUp
old_lookfrom = cam.lookFrom

start = time.time()

scale = 1
d = 15
steps = 120
cam.lookTo = vec3(0, 0, 0)
cam.lookUp = vec3(0, 1, 0)
for i in range(0, steps):
    r = (2 * math.pi * i) / (steps - 1)
    x = d * math.sin(r)
    z = -d * math.cos(r)
    cam.lookFrom = vec3(x * scale, 3 * scale, z * scale)
    inviwo_utils.update()  # Needed for canvas to update

for i in range(0, steps):
    r = (2 * math.pi * i) / (steps - 1)
    x = 1.0 * math.sin(r)
    z = 1.0 * math.cos(r)
    cam.lookUp = vec3(x * scale, z * scale, 0)
    inviwo_utils.update()  # Needed for canvas to update

end = time.time()
fps = 2 * steps / (end - start)

fps = round(fps, 3)

cam.lookTo = old_lookto
cam.lookUp = old_lookup
cam.lookFrom = old_lookfrom

print("Frames per second: " + str(fps))
print("Time per frame: " + str(round(1000 / fps, 1)) + " ms")
