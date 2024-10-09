import inviwo

if __name__ == '__main__':
    inviwo.setupPaths("C:/Inviwo/Output/bin/RelWithDebInfo")
    import inviwopy

    app = inviwo.Inviwo()
    app.load("<my workspace file>")

    app.mainWindow.setMinimumSize(800, 600)
    app.hideMenyAndToolBars()

    app.showProperties(["VolumeRaycaster"])
    app.setCanvasAsCentralWidget(app.inviwoApp.network.processors.Canvas)

    app.qapp.exec()