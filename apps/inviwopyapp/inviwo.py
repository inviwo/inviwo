import argparse
import pathlib
import os
import sys
import shiboken6
import PySide6
import PySide6.QtWidgets
import PySide6.QtGui

"""
    If python can't find the`inviwopy` module use the setupPaths to pass
    in the path to inviwo bin directory.

    If the program complains about missing QT plugins use setupPaths with
    the path to inviwo bin directory. It will set the 'QT_PLUGIN_PATH' in python.
    Or you can try and set it manually.
"""


def setupPaths(inviwoLibDir):
    sys.path.append(str(inviwoLibDir))
    os.environ['QT_PLUGIN_PATH'] = str(inviwoLibDir)


def configureQtApp():
    PySide6.QtCore.QCoreApplication.setAttribute(
        PySide6.QtCore.Qt.ApplicationAttribute.AA_NativeWindows)
    PySide6.QtCore.QCoreApplication.setAttribute(
        PySide6.QtCore.Qt.ApplicationAttribute.AA_ShareOpenGLContexts)
    PySide6.QtCore.QCoreApplication.setAttribute(
        PySide6.QtCore.Qt.ApplicationAttribute.AA_UseDesktopOpenGL)
    defaultFormat = PySide6.QtGui.QSurfaceFormat()
    defaultFormat.setMajorVersion(10)
    defaultFormat.setProfile(PySide6.QtGui.QSurfaceFormat.CoreProfile)
    PySide6.QtGui.QSurfaceFormat.setDefaultFormat(defaultFormat)


def configureQtNames(qtApp):
    qtApp.setOrganizationName("Inviwo Foundation")
    qtApp.setOrganizationDomain("inviwo.org")
    qtApp.setApplicationName("Inviwo")
    qtApp.setWindowIcon(PySide6.QtGui.QIcon(":/inviwo/inviwo_light.png"))


class Inviwo:
    def __init__(self):
        """
        Constructs an Inviwo application.
        """

        self.logCentral = None
        self.loggers = []
        self.qtApp = None
        self.inviwoApp = None
        self.propertyListWidget = None
        self.mainWindow = None

        try:
            import inviwopy
        except ModuleNotFoundError as e:
            print("Did you remember to add the path to 'inviwppy.*.pyd' to the PYTHONPATH?")
            print("Powershell: $Env:PYTHONPATH=\"<path to inviwo bin folder>\"")
            print("CMD: set PYTHONPATH=\"<path to inviwo bin folder>\"")
            print("BASH: export PYTHONPATH=\"<path to inviwo bin folder>\"")
            print("You can also use the libdir cli parameter to pass the path.")
            raise e

        self.inviwopy = inviwopy

        # Inviwo requires that a logcentral is created.
        self.logCentral = inviwopy.LogCentral()

        # Create and register a console logger
        cl = inviwopy.ConsoleLogger()
        self.logCentral.registerLogger(cl)
        self.loggers.append(cl)

        # Create a Qt application
        configureQtApp()
        self.qtApp = PySide6.QtWidgets.QApplication()
        self.mainWindow = PySide6.QtWidgets.QMainWindow()
        self.mainWindow.setObjectName("InviwoMainWindow")

        # Create the inviwo application
        self.inviwoApp = inviwopy.InviwoApplication()
        inviwopy.app = self.inviwoApp
        self.inviwoApp.setProgressCallback(lambda x: inviwopy.log(x))
        self.inviwoApp.registerRuntimeModules(lambda name: name not in ['glfw', 'webbrowser'])

        configureQtNames(self.qtApp)
        inviwopy.qt.configureFileSystemObserver(self.inviwoApp)
        inviwopy.qt.configurePostEnqueueFront(self.inviwoApp)
        inviwopy.qt.setStyleSheetFile(":/stylesheets/inviwo.qss")

        self.propertyListWidget = inviwopy.qt.PropertyListWidget(self.inviwoApp)
        self.qplw = shiboken6.wrapInstance(
            self.propertyListWidget.address(), PySide6.QtWidgets.QDockWidget)
        self.mainWindow.addDockWidget(PySide6.QtCore.Qt.RightDockWidgetArea, self.qplw)
        self.qplw.setFloating(False)

        self.mainWindow.show()

    def clearProperties(self):
        self.propertyListWidget.clearProperties()
        self.propertyListWidget.hide()

    def showProperties(self, properties):
        """
        :param properties: List of processor identifiers and property paths to show in the
        property list widget
        """
        for path in properties:
            if len(path.split('.')) == 1:
                if proc := self.inviwoApp.network.getProcessorByIdentifier(path):
                    self.propertyListWidget.addProcessorProperties(proc)
            else:
                if prop := self.inviwoApp.network.getProperty(path):
                    self.propertyListWidget.addPropertyWidgets(prop)
        self.propertyListWidget.show()

    def load(self, workspace=None):
        """
        :param workspace: Path to workspace to load, defaults load boron.inv
        """
        workspace = (workspace if workspace else
                     (pathlib.Path(self.inviwoApp.getPath(self.inviwopy.PathType.Workspaces)
                                   + "/boron.inv")))
        self.inviwoApp.network.load(workspace.absolute().as_posix())

    def screenshot(self, filename, canvas="Canvas"):
        """
        :param filename: Path to a output file for a screenshot of the 'Canvas' processor
        :param canvas: a canvas processor instance or a processor identifier
        """

        if isinstance(canvas, str):
            canvas = self.inviwoApp.network.getProcessorByIdentifier(canvas)

        if not canvas:
            self.inviwopy.logWarn("Could not find any canvas")
            return

        # Make sure the app is ready
        self.inviwopy.qt.waitForNetwork(self.inviwoApp)
        # Save a snapshot
        canvas.snapshot(filename)

    def print_processors(self):
        for p in self.inviwoApp.network.processors:
            print(f"{p.identifier:40} {p.classIdentifier:40}")

    def print_properties(self):
        for processor in self.inviwoApp.network.processors:
            for prop in processor.getPropertiesRecursive():
                print(f"{prop.displayName:20} {prop.classIdentifier:40} {prop.path:40}")

    def setCanvasAsCentralWidget(self, canvas):
        if isinstance(canvas, str):
            canvas = self.inviwoApp.network.getProcessorByIdentifier(canvas)

        if not canvas:
            self.inviwopy.logWarn("Could not find any canvas")
            return

        a = self.inviwopy.qt.address(canvas.widget)
        w = shiboken6.wrapInstance(a, PySide6.QtWidgets.QWidget)
        w.setWindowFlags(PySide6.QtCore.Qt.Widget)
        self.mainWindow.setCentralWidget(w)
        self.mainWindow.show()

    def hideMenyAndToolBars(self):
        self.mainWindow.setMinimumSize(800, 600)
        self.mainWindow.menuBar().hide()
        for t in self.mainWindow.findChildren(PySide6.QtWidgets.QToolBar):
            t.hide()


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

    parser.add_argument('-c', '--central_widget', type=str, help="Canvas processor identifier")
    parser.add_argument('--hide', action="store_true", help="Hide menu and toolbar items")

    parser.add_argument('--libdir', type=pathlib.Path, help='Path to inviowpy.*.pyd')

    return parser.parse_args()


if __name__ == '__main__':
    args = make_cmd_parser()

    if args.libdir:
        setupPaths(str(args.libdir))

    app = Inviwo()

    app.load(args.workspace)

    if args.properties:
        app.showProperties(args.properties)

    if args.list_processors:
        app.print_processors()

    if args.list_properties:
        app.print_properties()

    if args.central_widget:
        app.setCanvasAsCentralWidget(args.central_widget)

    if args.hide:
        app.hideMenyAndToolBars()

    if args.screenshot:
        app.screenshot(args.screenshot)

    if args.run and not args.ipython:
        # run the app event loop
        app.qtApp.exec()

    if args.ipython:
        if args.run:
            import IPython
            IPython.get_ipython().enable_gui('qt6')
        else:
            print("enter '%gui qt6' in ipython to start the event loop")
