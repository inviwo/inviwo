#Inviwo Python script 
import inviwopy


app = inviwopy.app
network = app.network

def doWork():
    if(network.VolumePathTracer2.enableRefinement.value):
        network.VolumePathTracer2.enableRefinement.value = False
    else:
        network.VolumePathTracer2.enableRefinement.value = True


def generateReference():

def runTests():

def compareResults():


doWork()


# What do we need to test.
# All the

# Generate reference

# 


