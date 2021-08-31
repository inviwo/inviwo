import argparse
import pathlib
import os
import sys


def make_cmd_parser():
    parser = argparse.ArgumentParser(
        description="Run inviwopyapp",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-w', '--workspace', type=pathlib.Path, help='Workspace to load')
    parser.add_argument('-s', '--screenshot', type=pathlib.Path, help='Save screenshot to file')
    parser.add_argument("-r", "--run", action="store_true", help="Run event loop")
    parser.add_argument("-i", "--ipython", action="store_true",
                        help="Enable ipython event loop hook")
    parser.add_argument('-p', '--properties', type=str, nargs='*', help='Paths to properties')
    parser.add_argument("--list_processors", action="store_true",
                        help="List all processors in the workspace")
    parser.add_argument('--list_properties', type=str, nargs='*',
                        help='List properties of processor')
    parser.add_argument('--libdir', type=pathlib.Path, help='Path to inviowpy.*.pyd')

    return parser.parse_args()


def main(workspace=None, properties=None, screenshot=None, run=False, ipython=False,
         list_processors=False, list_properties=None):
    """ Constructs a Inviwo application and loads a workspace.
        
    :param workspace: Path to workspace to load, defaults load boron.inv
    :param properties: List of processor identifiers and property paths to show in the 
        property list widget
    :param screenshot: Path to a output file for a screenshot of the 'Canvas' processor
    :paran run: Start the qt event loop, default False
    :param ipython: Register the ipython event loop hook. Use the ipython magic command
        '#gui inviwo' to start.
    :param list_processors: Output a list of all processors, default False.
    :param list_properties: Output a list of all properties of the Processors identifiers given


    If python can't find the i`inviwopy` module use the '--libdir' CLI argument to pass
    in the path to inviwo bin directory.  

    If the program complains about missing QT plugins try passing the '--libdir'
    CLI argument. It will set the 'QT_PLUGIN_PATH' in python. Or you can try and
    set it manually.
    """

    try:
        import inviwopy as ivw
        import inviwopyapp as qt
    except ModuleNotFoundError as e:
        print("Did you remember to add the path to 'inviwppy.*.pyd' to the PYTHONPATH?")
        print("Powershell: $Env:PYTHONPATH=\"<path to inviwo bin folder>\"")
        print("CMD: set PYTHONPATH=\"<path to inviwo bin folder>\"")
        print("BASH: export PYTHONPATH=\"<path to inviwo bin folder>\"")
        print("You can also use the libdir cli parameter to pass the path.")
        raise e

    def addIpythonGuiHook(inviwoApp: qt.InviwoApplicationQt, name: str = "inviwo") -> bool:
        """ Add a event loop hook in IPyhton to enable simultanious use of the IPython terminal
        and the inviwo qt gui use the IPython magic function '#gui inviwo' to start the event
        loop after calling this function.
        See https://ipython.readthedocs.io/en/stable/config/eventloops.html
        """
        try:
            import IPython
        except ImportError:
            return False

        def inputhook(context):
            while not context.input_is_ready():
                inviwoApp.update()
        IPython.terminal.pt_inputhooks.register(name, inputhook)

        return True

    res = []

    # Inviwo requires that a logcentral is created.
    lc = ivw.LogCentral()
    res.append(lc)

    # Create and register a console logger
    cl = ivw.ConsoleLogger()
    lc.registerLogger(cl)

    # Create the inviwo application
    try:
        app = qt.InviwoApplicationQt()
    except ... as e:
        print("failed to create app")
        print(e)
        raise e

    res.append(app)
    app.registerModules()

    # load a workspace
    workspace = (workspace if workspace else
                 (app.getPath(ivw.PathType.Workspaces) + "/boron.inv"))
    app.network.load(workspace)

    if properties:
        plw = ivw.qt.PropertyListWidget(app)
        res.append(plw)
        for path in properties:
            if len(path.split('.')) == 1:
                if proc := app.network.getProcessorByIdentifier(path):
                    plw.addProcessorProperties(proc)
            else:
                if prop := app.network.getProperty(path):
                    plw.addPropertyWidgets(prop)

        plw.move(100, 100)
        plw.show()

    if list_processors:
        print("Processors:")
        for p in app.network.processors:
            print(f"{p.identifier:40} {p.classIdentifier:40}")

    if list_properties:
        for identifier in list_properties:
            processor = app.network.getProcessorByIdentifier(identifier)
            if processor:
                print(f"Properties for {identifier}")
                for prop in processor.getPropertiesRecursive():
                    print(f"{prop.displayName:20} {prop.classIdentifier:40} {prop.path:40}")

    if screenshot:
        # Make sure the app is ready
        app.update()
        app.waitForPool()
        app.update()
        # Save a snapshot
        app.network.Canvas.snapshot(screenshot)

    if run:
        # run the app event loop
        app.run()

    if ipython:
        addIpythonGuiHook(app)
        print("enter '#gui inviwo' in ipython to start the event loop")

    return app


if __name__ == '__main__':
    args = make_cmd_parser()

    if args.libdir:
        sys.path.append(str(args.libdir))
        os.environ['QT_PLUGIN_PATH'] = str(args.libdir)

    main(workspace=args.workspace,
         properties=args.properties,
         screenshot=args.screenshot,
         run=args.run,
         ipython=args.ipython,
         list_processors=args.list_processors,
         list_properties=args.list_properties)
