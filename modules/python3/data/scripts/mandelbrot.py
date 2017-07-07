import inviwopy
from inviwopy.glm import *
import numpy as np
import math

# input variables 
# img - memory for the final image
# p - the processor

rAxis = np.linspace(p.realBounds.value[0],p.realBounds.value[1],img.data.shape[0])
iAxis = np.linspace(p.imaginaryBound.value[0],p.imaginaryBound.value[1],img.data.shape[1])


po = p.power.value
its = p.iterations.value

for (index,v) in np.ndenumerate(img.data):  
    C = Z = complex( rAxis[index[0]] , iAxis[index[1]] ); 
    for i in range(0,its):
        if(abs(Z)>2):
            img.data[index[0],index[1]] = math.log(1+i); 
            break;
        Z = np.power(Z,po) + C;