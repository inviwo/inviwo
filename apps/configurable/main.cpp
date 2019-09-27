#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include "gui.h"

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspacemanager.h>

#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/consolelogger.h>

#include <inviwo/core/moduleregistration.h>

int main(int argc, char** argv) {

    using namespace inviwo;

    LogCentral::init();
    inviwo::util::OnScopeExit deleteLogcentral([]() { inviwo::LogCentral::deleteInstance(); });
    auto logger = std::make_shared<inviwo::ConsoleLogger>();
    LogCentral::getPtr()->registerLogger(logger);

    InviwoApplicationQt inviwoApp(argc, argv, "Inviwo-Qt");
    inviwoApp.printApplicationInfo();
    inviwoApp.setAttribute(Qt::AA_NativeWindows);
    inviwoApp.setProgressCallback([](std::string m) {
        LogCentral::getPtr()->log("InviwoApplication", LogLevel::Info, LogAudience::User, "", "", 0,
                                  m);
    });

    inviwoApp.setWindowIcon(QIcon(":/inviwo/inviwo_light.png"));
    inviwoApp.setAttribute(Qt::AA_NativeWindows);
    QFile styleSheetFile(":/stylesheets/inviwo.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QString::fromUtf8(styleSheetFile.readAll());
    inviwoApp.setStyleSheet(styleSheet);
    styleSheetFile.close();

    // Initialize all modules
    inviwoApp.registerModules(inviwo::getModuleList());

    auto& cmdparser = inviwoApp.getCommandLineParser();
    TCLAP::ValueArg<std::string> snapshotArg(
        "s", "snapshot",
        "Specify default name of each snapshot, or empty string for processor name.", false, "",
        "Snapshot default name: UPN=Use Processor name.");

    cmdparser.add(&snapshotArg,
                  [&]() {
                      std::string path = cmdparser.getOutputPath();
                      if (path.empty()) path = inviwoApp.getPath(PathType::Images);
                      util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path,
                                            snapshotArg.getValue());
                  },
                  1000);

    // Do this after registerModules if some arguments were added
    cmdparser.parse(inviwo::CommandLineParser::Mode::Normal);

    inviwoApp.getProcessorNetwork()->lock();

    const std::string workspace = cmdparser.getLoadWorkspaceFromArg()
                                      ? cmdparser.getWorkspacePath()
                                      : inviwoApp.getPath(PathType::Workspaces, "/boron.inv");

    if (!loadWorkspace(workspace, inviwoApp.getWorkspaceManager())) {
        QMessageBox msgBox;
        msgBox.setText("Workspace could not be loaded. Infos in console.");
        msgBox.exec();
    }

    ConfigGUI gui(&inviwoApp);

    inviwoApp.processFront();
    inviwoApp.getProcessorNetwork()->unlock();

    cmdparser.processCallbacks();  // run any command line callbacks from modules.

    if (cmdparser.getQuitApplicationAfterStartup()) {
        inviwoApp.closeInviwoApplication();
        inviwoApp.quit();
        return 0;
    } else {
        return inviwoApp.exec();
    }
}