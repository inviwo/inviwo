import os
import os.path

def ensureDirectory(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)

def update():
    try:
        import inviwoqt as qt
        qt.update();
    except: 
        pass


def snapshotAllCanvasesWithWorkspace(basePath: str , workspaceName  , canvasFilenamePrefix="" , canvasFilenameSufix = "" , filetype="png"):
    import inviwo as i
    canvases = [];
    for canvas in i.listCanvases():
        canvases.append(canvas[0])
    return snapshotWithWorkspace(basePath , canvases, workspaceName, canvasFilenamePrefix, canvasFilenameSufix , filetype )



def snapshotWithWorkspace(basePath: str, canvases , workspaceName  , canvasFilenamePrefix="" , canvasFilenameSufix = "" , filetype="png"):
    canvasList = []
    if type(canvases) is list:
        canvasList = canvases
    elif type(canvases) is str:
        canvasList = [canvases]
    else:
        raise TypeError("snapshotWithWorkspace expect parameter 2 to be either string or a list of strings of canvas ids")


    import inviwo
    import inviwoqt

    if not workspaceName.endswith('.inv'):
        workspaceName = workspaceName + '.inv'

    inviwoqt.saveWorkspace(basePath + "/" + workspaceName );

    workspacePath = basePath + "/" + workspaceName;
    workspaceDir = os.path.dirname(workspacePath);
    
    ensureDirectory(basePath)
    ensureDirectory(workspaceDir)

    inviwoqt.saveWorkspace(basePath + "/" + workspaceName  , False);
    for c in canvasList:
        inviwo.snapshot(basePath + "/" + canvasFilenamePrefix + c + canvasFilenameSufix + "." + filetype , c );
