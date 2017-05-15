import os
import os.path

def ensureDirectory(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)

def update():
    try:
        from inviwopy import qt
        qt.update();
    except: 
        pass

def getCanvases():
    import inviwopy as i
    return i.app.network.canvases

def snapshotAllCanvasesWithWorkspace(basePath: str , workspaceName  , canvasFilenamePrefix="" , canvasFilenameSufix = "" , filetype="png"):
    import inviwopy as i
    ensureDirectory(basePath);
    return snapshotWithWorkspace(basePath , i.app.network.canvases, workspaceName, canvasFilenamePrefix, canvasFilenameSufix , filetype )



def snapshotWithWorkspace(basePath: str, canvases , workspaceName  , canvasFilenamePrefix="" , canvasFilenameSufix = "" , filetype="png"):
    
    import inviwopy
    
    from inviwopy import CanvasProcessor

    canvasList = []
    if type(canvases) is list:
        canvasList = canvases
    elif type(canvases) is CanvasProcessor:
        canvasList = [canvases]
    else:
        raise TypeError("snapshotWithWorkspace expect parameter 2 to be either a CanvasProcessor or a list of strings of canvas ids")


    workspaceName = workspaceName.strip();
    canvasFilenamePrefix = canvasFilenamePrefix.strip();
    canvasFilenameSufix = canvasFilenameSufix.strip();

    if not workspaceName.endswith('.inv'):
        workspaceName += '.inv'


    workspacePath = basePath + "/" + workspaceName;
    workspaceDir = os.path.dirname(workspacePath);
    
    ensureDirectory(basePath)
    ensureDirectory(workspaceDir)

    inviwopy.app.network.save(basePath + "/" + workspaceName);
    for c in canvasList:
        c.snapshot(basePath + "/" + canvasFilenamePrefix + c.identifier + canvasFilenameSufix + "." + filetype );
