import inviwopy as ivw
import inviwopyapp as qt

if __name__ == '__main__':
    # Inviwo requires that a logcentral is created.
    lc = ivw.LogCentral()
    
    # Create and register a console logger
    cl = ivw.ConsoleLogger()
    lc.registerLogger(cl)

    # Create the inviwo application
    app = qt.InviwoApplicationQt()
    app.registerModules()

    # load a workspace
    app.network.load(app.getPath(ivw.PathType.Workspaces) + "/boron.inv")
    
    print("Load processors:")
    for p in app.network.processors:
        print(p.identifier)

    plw = ivw.qt.PropertyListWidget(app)
    plw.addProcessorProperties(app.network.VolumeRaycaster)
    plw.addProcessorProperties(app.network.VolumeSource)
    plw.move(100,100)
    plw.show()

    # Make sure the app is ready
    app.update()
    app.waitForPool()
    app.update()
    # Save a snapshot
    app.network.Canvas.snapshot("snapshot.png") 

    # run the app event loop
    app.run()