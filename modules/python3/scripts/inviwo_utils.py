

def update():
    try:
        import inviwoqt as qt
        qt.update();
    except: 
        pass


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
    for c in canvasList:
        print(c)
        inviwo.snapshot(basePath + "/" + canvasFilenamePrefix + c + canvasFilenameSufix + "." + filetype , c );
