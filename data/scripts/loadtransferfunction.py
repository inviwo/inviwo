# Inviwo Python script 
import inviwopy

tf = inviwopy.app.network.VolumeRaycaster.transferFunction
tf.load( inviwopy.app.getPath( inviwopy.PathType.TransferFunctions) + "/transferfunction.itf" )

