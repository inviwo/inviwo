import inviwopy
from inviwopy.glm import *
inviwopy.app.network.Background.color1.set(vec4(1,1,0,1))
inviwopy.app.network.Background.color2.set(vec4(1,0,1,1))

print(inviwopy)
print(inviwopy.app)
print(inviwopy.app.network)
print(inviwopy.app.network.VolumeRaycaster)
print(inviwopy.app.network.VolumeRaycaster.raycaster)
print(inviwopy.app.network.VolumeRaycaster.raycaster.samplingRate)
inviwopy.app.network.VolumeRaycaster.raycaster.getPropertyByIdentifier("samplingRate").set(2)
#inviwopy.app.network.VolumeRaycaster.raycaster.samplingRate.set(2)

