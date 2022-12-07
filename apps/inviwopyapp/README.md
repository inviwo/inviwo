# Inviwo python app example

```py
import inviwo

if __name__ == '__main__':
    inviwo.setupPaths("<path to the inviwo bin dir>")
    import inviwopy

    app = inviwo.Inviwo()
    app.load("<my workspace file>")

    app.mainWindow.setMinimumSize(800, 600)
    app.hideMenyAndToolBars()

    app.showProperties(["VolumeRaycaster"])
    app.setCanvasAsCentralWidget(app.inviwoApp.network.processors.Canvas)

    app.qapp.exec()
```