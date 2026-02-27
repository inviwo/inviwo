#Inviwo Python script

import inviwopy

import ivw.utils as inviwo_utils

from inviwopy.glm import ivec2, size2_t

import time

import os

import numpy as np

import subprocess

app = inviwopy.app

network = app.network

windowRes = ivec2(512, 512)

referenceName = "Reference_N_50000.tiff"
referenceNameNoUpdate = "Reference_N_50000_noUpdate.tiff"

outputdir = "/home/henry/Projects/Exjobb/testoutput"
workspacedir = "/home/henry/Projects/Exjobb/workspaces"

workspaces = ["boronTestWorkspace", "bunny_cloudTestWorkspace", "fireTestWorkspace", "smoke2TestWorkspace", "SubclaviaTestWorkspace"]

allowedComputationTime = 2.5

UGridCellSizes = [1, 2, 4, 8, 16, 32, 64]
#UGridCellSizes = [16]

def showOtherCanvases():
    visibility = True

    network.Canvas.widget.visible = visibility

    #network.AbsDiff.widget.visibility = visibility

    #network.Diff.widget.visibility = visibility

    #network.ColorMappedDifference.widget.visibility = visibility

    #network.Subset.widget.visibility = visibility

def hideOtherCanvases():
    visibility = False

    network.Canvas.widget.visible = visibility

    #network.AbsDiff.widget.visibility = visibility

    #network.Diff.widget.visibility = visibility

    #network.ColorMappedDifference.widget.visibility = visibility

    #network.Subset.widget.visibility = visibility

def snapshotCanvases(outputdir, basename):

    showOtherCanvases()

    network.Canvas.snapshot('{}/{}.tiff'.format(outputdir, basename))

    network.Canvas.snapshot('{}/{}.png'.format(outputdir, basename))

    #network.AbsDiff.snapshot('{}/AbsDiff_{}.tiff'.format(outputdir, basename))

    #network.Diff.snapshot('{}/Diff_{}.tiff'.format(outputdir, basename))

    #network.ColorMappedDifference.snapshot('{}/Diff_{}.png'.format(outputdir, basename))

    #network.Subset.snapshot('{}/Subset_{}.png'.format(outputdir, basename))

    hideOtherCanvases()

# What picture are we making a reference of?
# Do we make a workspace for each dataset?
def generateReference(outputdir, dataset):
    inviwopy.logInfo("generateReference called")

    showOtherCanvases()

    iterate = network.VolumePathTracer.iterate

    transmittanceMethod = network.VolumePathTracer.transmittanceMethod

    network.VolumePathTracer.region.value = 16 # 

    transmittanceMethod.selectedIndex = 0 # Woodcock tracking

    allowedComputeTime = 60

    nIterations = 0

    nSamples = 50000 # 50000 is resonable

    estimatedComputeTime = 0

    start = time.perf_counter()

    prevIterationTime = start

    #while ((time.perf_counter() - start)+0.5*estimatedComputeTime) < allowedComputeTime:

    for i in range(nSamples):

        iterate.press()

        #inviwo_utils.update() # Needed for canvas to update, makes it take much longer, but might be needed for a render.

        estimatedComputeTime = time.perf_counter() - prevIterationTime

        prevIterationTime = time.perf_counter()

        nIterations += 1


    end = time.perf_counter()

    elapsedTime = (end - start)/nIterations

    print(transmittanceMethod.selectedDisplayName + " (" + str(nIterations) + "): " + str(elapsedTime*1000) + " ms")

    basename = "Reference_N_" + str(nIterations)
    #basename = "Reference_WithUpdate_N_" + str(nIterations)

    snapshotCanvases(outputdir + '/' + dataset, basename)


def imageCompare(refPath, compWithPath, metric):
    cmd = ['compare', '-metric', metric, refPath, compWithPath, os.path.splitext(compWithPath)[0] + '.png']

    proc = subprocess.run(args=cmd, encoding='utf-8', stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    return proc.stderr


def testMethods(outputdir, dataset):
   

    iterate = network.VolumePathTracer.iterate
    
    invalidate = network.VolumePathTracer.getPropertyByIdentifier('invalidate')
    
    transmittanceMethod = network.VolumePathTracer.transmittanceMethod

    #showOtherCanvases()

    #possionTrackingN = network.VolumePathtracer.possionTrackingN

    #poissonUpperBoundMultiplier = network.VolumePathtracer.poissonUpperBoundMultiplier

    #network.UniformGridOpacityGL.region.value = 5

    file = open(outputdir + '/' + dataset + '/' + 'Log', 'a', encoding="utf-8")

    file.write(time.asctime(time.localtime()) + '\n')
    s = 'Dataset, Method, N, cellSize, executionTime (ms), RMSE, PSNR'
    file.write(s + '\n')
    print(s)

    for method in range(0, 7, 1):

        transmittanceMethod.selectedIndex = method

        #nIterationsBest = setBestParameters(isCPU, dataset, method)
        nIterationsBest = 0

        hideOtherCanvases()

        for cellSize in UGridCellSizes:

            #inviwo_utils.update()
            invalidate.press()
            
            network.VolumePathTracer.region.value = cellSize

            nIterations = 0

            estimatedComputeTime = 0

            start = time.perf_counter()

            prevIterationTime = start


            while ((time.perf_counter() - start)+0.5*estimatedComputeTime) < allowedComputationTime:

                iterate.press()    

                #inviwo_utils.update() # Needed for canvas to update

                estimatedComputeTime = time.perf_counter() - prevIterationTime

                prevIterationTime = time.perf_counter()

                nIterations += 1
       

            end = time.perf_counter()

            elapsedTime = (end - start)/nIterations

            #print(transmittanceMethod.selectedDisplayName + " (" + str(nIterations) + "): " + str(elapsedTime*1000) + " ms")

            basename = transmittanceMethod.selectedDisplayName

            snapshotCanvases(outputdir + '/' + dataset, basename + '_' + str(cellSize))
        
            rmse = imageCompare(outputdir + '/' + dataset + '/' + referenceName, outputdir + '/' + dataset + '/' + basename + '_' + str(cellSize) + '.tiff', 'RMSE').split()[0]
            psnr = imageCompare(outputdir + '/' + dataset + '/' + referenceName, outputdir + '/' + dataset + '/' + basename + '_' + str(cellSize) + '.tiff', 'PSNR').split()[0]
            log = '{}, {}, {}, {}, {}, {}, {}'.format(dataset, transmittanceMethod.selectedDisplayName, nIterations, cellSize, elapsedTime*1000, rmse, psnr)
            print(log)
            file.write(log + '\n')
    
    file.write('\n')
    file.close()

# Sets up the workspace to do tests with; initializes properties so on so forth
def runTests(ws):
    inviwopy.logInfo("runTests called")
    network.load(workspacedir + "/" +  ws + ".inv")

    dataset = os.path.basename(network.VolumeSource.filename.value)
    

    # changing wrapping breaks the renders for some stupid reason. im so tired of this
    #network.VolumeSource.Information.xWrappingX.selectedIndex = 0
    #network.VolumeSource.Information.yWrappingX.selectedIndex = 0
    #network.VolumeSource.Information.zWrappingX.selectedIndex = 0
    
    #network.Canvas.inputSize.dimensions.value = windowRes

    generateReference(outputdir, dataset)
    # Test all transmittance functions
    # Possible loop to test region size
    testMethods(outputdir, dataset)



# What do we test? 
# all 7 transmittance methods
# We can test with different region sizes of the uniform grid
# We can have space for these two to be set as arguments

# Problem im stumbling across with these tests is that if the canvas isn't being shown, then my processor doesn't render to it.
# I wonder if it can easily be fixed by just adding an Image as a member variable to my own process that we draw to, and then just send it as output.
# That way we don't constantly ask for an image rep from our outport (maybe thats how it works idfk)

# Because of the instability of the opengl context when opening a new workspace, I choose to do it manually. 
# Untill atleast I've checked that the workspace can be opened flawlessly

runTests(workspaces[0])