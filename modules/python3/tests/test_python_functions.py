# Inviwo Python script 
import inviwo 
import inviwoqt 

#help('modules')


#help('inviwo')

outpath = "D:/temp/pythontest/"

ws = inviwo.getDataPath() + "/workspaces/boron.inv"
inviwoqt.loadWorkspace(ws)

inviwo.wait() 

inviwo.snapshotCanvas(0,outpath +"/snapshot001.png")

inviwo.setPropertyValue("CubeProxyGeometry.clipX",(75,149))

inviwo.snapshotAllCanvases(outpath, "snapshots" , "png")
inviwo.snapshotAllCanvases(outpath)


sampRate = inviwo.getPropertyValue("VolumeRaycaster.raycaster.samplingRate");
if sampRate != 3.0:
    print("should not get here" , file=sys.stderr)
