# Inviwo Python script 
import inviwopy

tf = inviwopy.app.network.VolumeRaycaster.transferFunction
tf.save( inviwopy.app.getPath( inviwopy.PathType.TransferFunctions) + "/transferfunction.itf" )
