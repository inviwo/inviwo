#Inviwo Python script
import inviwopy
import numpy as np


app = inviwopy.app
net = app.network

vm = np.array(net.VolumeRaycaster.camera.value.viewMatrix)
pm = np.array(net.VolumeRaycaster.camera.value.projectionMatrix)

cameraPos = np.array(net.VolumeRaycaster.camera.value.lookFrom)


def computeNearPlane(x,y):
    clipSpaceCoord = np.array([x,y,-1,1])
    viewSpaceCoord = np.matmul(np.linalg.inv(pm),clipSpaceCoord)
    viewSpaceCoord /= viewSpaceCoord[3]

    worldSpaceCoord = np.matmul(np.linalg.inv(vm),viewSpaceCoord)  
    return worldSpaceCoord

def computeFarPlane(x,y):
    clipSpaceCoord = np.array([x,y,1,1])
    viewSpaceCoord = np.matmul(np.linalg.inv(pm),clipSpaceCoord)
    viewSpaceCoord /= viewSpaceCoord[3]

    worldSpaceCoord = np.matmul(np.linalg.inv(vm),viewSpaceCoord)  
    return worldSpaceCoord




loopPar = np.array([0,0,1,1])

step_size = 0.1

for xl in np.arange(loopPar[0], loopPar[2] + step_size, step_size):
#    for yl in np.arange(loopPar[1], loopPar[3] + step_size, step_size):
        # You can access the current x and y values in the loop
        
    x = 0.5 * 2.0 - 1.0#;  // Normalizing to clip space
    y = 1 - 0.5 * 2.0#;  // Y-flip for OpenGL
    nearPlane = computeNearPlane(x,y)
    farPlane = computeFarPlane(x,y)
    print("nearPlane", nearPlane )
    print("\nFerPlane", farPlane )

    direction = farPlane - nearPlane;

    length = np.linalg.norm(direction)
    print("\nlength is:",length )


#print(np.linalg.inv(vm))
#print(numpy.matmul(np.linalg.inv(vm), vm))