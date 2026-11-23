# Inviwo Python script
import inviwopy

from inviwopy.glm import size2_t, dvec3,dvec2
from inviwopy import resource

import math
import time

import ivw.regression
import ivw.camera

class ResourceObserver(inviwopy.ResourceManagerObserver):
    def __init__(self):
        inviwopy.ResourceManagerObserver.__init__(self)
        self.used = {resource.RAM.group:0, resource.GL.group: 0, resource.PY.group: 0}
        self.curr = {resource.RAM.group:0, resource.GL.group: 0, resource.PY.group: 0}
        
    def reset(self):
        self.used = {resource.RAM.group:0, resource.GL.group: 0, resource.PY.group: 0}
        self.curr = {resource.RAM.group:0, resource.GL.group: 0, resource.PY.group: 0}

    def onDidAddResource(self, group, index, r):
        if r.dims[0] != 0 and r.dims[0] != 0 and r.dims[2] != 0:
            self.curr[group] += r.sizeInBytes()
            self.used[group] += r.sizeInBytes()

    def onDidUpdateResource(self, group, index, r):
        if r.dims[0] != 0 and r.dims[0] != 0 and r.dims[2] != 0:
            self.used[group] += r.sizeInBytes()

    def onDidRemoveResource(self, group, index, r):
        if r.dims[0] != 0 and r.dims[0] != 0 and r.dims[2] != 0:
            self.curr[group] -= r.sizeInBytes()

m = ivw.regression.Measurements()

network = inviwopy.app.network

def loadSameDim(i):
    network.HDF5ToVolume.dataRangeGroup.customValueRange.value = dvec2(0,i + 1)

def loadNewDims(i):
    network.HDF5ToVolume.outputGroup.selection.dim00.start.value = i

def harness(testFunc, steps, update):
    start = time.perf_counter()
    for i in range(0, steps):
        network.lock()
        testFunc(i)
        network.unlock()
        if update: inviwopy.qt.update()
    end = time.perf_counter()
    return (end - start) / steps

steps = 10

workspace = inviwopy.app.workspaceManager.saveToString()
inviwopy.app.workspaceManager.clear()
inviwopy.qt.update()

observer = ResourceObserver()
inviwopy.app.resourceManager.addObserver(observer)

print(f"{'Test ':15} {'VolumeHDF':10} {'Update Qt':10} {'Time [ms]':>10} {'Used: RAM':>14} {'Used: GL':>14} {'Curr: RAM':>14} {'Curr: GL':>14}")

for test in [loadSameDim, loadNewDims]:
    for update in [False, True]:
        for lazy in [False, True]:
            network.lock()

            observer.reset()                                      
       
            inviwopy.app.workspaceManager.loadFromString(workspace)
            network.HDF5ToVolume.outputGroup.lazyLoading.value = lazy 

            network.unlock()
            inviwopy.qt.update()

            result = harness(test, steps, update)

            inviwopy.app.workspaceManager.clear()
            inviwopy.qt.update()

            print(f"{test.__name__:15} {lazy!s:10} {update!s:10} {1000*result :10.5}"
                  f" {observer.used[resource.RAM.group]:14,} {observer.used[resource.GL.group]:14,}"
                  f" {observer.curr[resource.RAM.group]:14,} {observer.curr[resource.GL.group]:14,}")
            m.addTime(f"Time-{test.__name__}-VolumeHDF:{lazy!s}-QtUpdate:{update!s}", result)
            m.addCount(f"RAM-Used-{test.__name__}-VolumeHDF:{lazy!s}-QtUpdate:{update!s}", observer.used[resource.RAM.group])
            m.addCount(f"GL-Used-{test.__name__}-VolumeHDF:{lazy!s}-QtUpdate:{update!s}", observer.used[resource.GL.group])
            m.addCount(f"RAM-Curr-{test.__name__}-VolumeHDF:{lazy!s}-QtUpdate:{update!s}", observer.curr[resource.RAM.group])
            m.addCount(f"GL-Curr-{test.__name__}-VolumeHDF:{lazy!s}-QtUpdate:{update!s}", observer.curr[resource.GL.group])
            inviwopy.qt.update()            

m.save()

inviwopy.app.workspaceManager.loadFromString(workspace)
inviwopy.qt.update()