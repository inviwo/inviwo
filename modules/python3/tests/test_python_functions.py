# Inviwo Python script 
import inviwo 
import inviwoqt 

ws = inviwo.getDataPath() + "/workspaces/boron.inv"
inviwoqt.loadWorkspace(ws)

inviwo.setPropertyValue("CubeProxyGeometry.clipX",75)