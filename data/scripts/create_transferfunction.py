# Inviwo Python script 
import inviwo 

def color(x):
   r = x
   g = 1-x
   return (r,g,0)

inviwo.clearTransferfunction("VolumeRaycaster.transferFunction")
inviwo.addPointToTransferFunction("VolumeRaycaster.transferFunction",(0.0,0.0),(0,0,0)) 

for i in range(1,256,9):
   x = i / 256.0
   a = 0
   if i%2==1:
      a = 0.1
   inviwo.addPointToTransferFunction("VolumeRaycaster.transferFunction",(x,a),color(x)) 


inviwo.addPointToTransferFunction("VolumeRaycaster.transferFunction",(1.0,0.0),(0,0,0)) 
