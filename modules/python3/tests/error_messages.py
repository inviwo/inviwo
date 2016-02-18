#Inviwo Python script 
import inviwo 
import inviwoqt 
import sys
import traceback

def printError(err):
    (exc_type, exc_value, exc_traceback) = err
    print(exc_value)
    traceback.print_tb(exc_traceback, limit=1, file=sys.stdout)
    print("")

def expectError():
    sys.exit()


###################
#### Load a workspsace ###
###################

#inviwoqt.loadWorkspace(inviwo.getDataPath() + "/workspaces/boron.inv")


#########################
### TEST PROPERTY FUNCTIONS ####
############################


try:
    inviwo.setPropertyValue(12342,12342)
except:
    printError(sys.exc_info())    

try:
    inviwo.setPropertyValue("no.property.with.this.path",12342)
except:
    printError(sys.exc_info())    


try:
    inviwo.setPropertyValue("CubeProxyGeometry.clipX",12342)
except:
    printError(sys.exc_info())    

try:
    inviwo.setPropertyValue("CubeProxyGeometry.clipX","asdf")
except:
    printError(sys.exc_info())    

try:
    inviwo.setPropertyValue("VolumeRaycaster.transferFunction","asdf")
except:
    printError(sys.exc_info())    