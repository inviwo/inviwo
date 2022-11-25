/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/qt/applicationbase/qtapptools.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/moduleregistration.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/networkdebugobserver.h>

#include <QApplication>
#include <QMainWindow>

int main(int argc, char** argv) {
    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);

    auto clogger = std::make_shared<inviwo::ConsoleLogger>();
    inviwo::LogCentral::getPtr()->registerLogger(clogger);

    // Must be set before constructing QApplication
    inviwo::utilqt::configureInviwoQtApp();

#ifdef IVW_DEBUG
    qInstallMessageHandler(&inviwo::utilqt::logQtMessages);
#endif

    QApplication qtApp{argc, argv};
    inviwo::InviwoApplication inviwoApp(argc, argv, "Inviwo-Qt");


    inviwo::utilqt::configureInviwoDefaultNames();
    inviwo::utilqt::configureFileSystemObserver(inviwoApp);
    inviwo::utilqt::configurePostEnqueueFront(inviwoApp);
    inviwo::utilqt::setStyleSheetFile(":/stylesheets/inviwo.qss");
    inviwoApp.setUILocale(inviwo::utilqt::getCurrentStdLocale());

    inviwoApp.printApplicationInfo();
    LogInfoCustom("InviwoInfo", "Qt Version " << QT_VERSION_STR);

    inviwoApp.setProgressCallback([](std::string_view m) {
        inviwo::LogCentral::getPtr()->log("InviwoApplication", inviwo::LogLevel::Info,
                                          inviwo::LogAudience::User, "", "", 0, m);
    });

    // Initialize all modules
    inviwoApp.registerModules(inviwo::getModuleList());

    auto& cmdparser = inviwoApp.getCommandLineParser();
    TCLAP::ValueArg<std::string> snapshotArg(
        "s", "snapshot",
        "Specify default name of each snapshot, or empty string for processor name.", false, "",
        "Snapshot default name: UPN=Use Processor name.");

    cmdparser.add(
        &snapshotArg,
        [&]() {
            std::string path = cmdparser.getOutputPath();
            if (path.empty()) path = inviwoApp.getPath(inviwo::PathType::Images);
            inviwo::util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path,
                                          snapshotArg.getValue());
        },
        1000);

    TCLAP::SwitchArg debugProcess("d", "debug",
                                  "Add debug logging for processor evaluation to the log");

    inviwo::NetworkDebugObserver obs;
    cmdparser.add(
        &debugProcess,
        [&]() {
            inviwoApp.getProcessorNetwork()->addObserver(&obs);
            inviwoApp.getProcessorNetworkEvaluator()->addObserver(&obs);
            inviwoApp.getProcessorNetwork()->forEachProcessor(
                [&](auto* p) { p->ProcessorObservable::addObserver(&obs); });
        },
        200);

    TCLAP::SwitchArg fullscreenArg("f", "fullscreen", "Specify fullscreen if only one canvas");

    cmdparser.add(&fullscreenArg, [&]() {
        auto network = inviwoApp.getProcessorNetwork();

        std::vector<inviwo::ProcessorWidget*> widgets;
        network->forEachProcessor([&](inviwo::Processor* p) {
            if (p->isSink()) {
                if (auto widget = p->getProcessorWidget()) {
                    if (widget->isVisible()) {
                        widgets.push_back(widget);
                    }
                }
            }
        });

        if (widgets.size() == 1) {
            widgets[0]->setFullScreen(true);
        }
    });

    // Do this after registerModules if some arguments were added
    cmdparser.parse(inviwo::CommandLineParser::Mode::Normal);

    QMainWindow mainWin;

    // Need to clear the network and (will delete processors and processorwidgets)
    // before QMainWindoes is deleted, otherwise it will delete all processorWidgets
    // before Processor can delete them.
    inviwo::util::OnScopeExit clearNetwork([&]() { inviwoApp.getProcessorNetwork()->clear(); });

    // Load workspace
    inviwoApp.getProcessorNetwork()->lock();

    const std::string workspace =
        cmdparser.getLoadWorkspaceFromArg()
            ? cmdparser.getWorkspacePath()
            : inviwoApp.getPath(inviwo::PathType::Workspaces, "/boron.inv");

    try {
        if (!workspace.empty()) {
            inviwoApp.getWorkspaceManager()->load(workspace, [&](inviwo::ExceptionContext ec) {
                try {
                    throw;
                } catch (const inviwo::IgnoreException& e) {
                    inviwo::util::log(
                        e.getContext(),
                        "Incomplete network loading " + workspace + " due to " + e.getMessage(),
                        inviwo::LogLevel::Error);
                }
            });
        }
    } catch (const inviwo::AbortException& exception) {
        inviwo::util::log(
            exception.getContext(),
            "Unable to load network " + workspace + " due to " + exception.getMessage(),
            inviwo::LogLevel::Error);
        return 1;
    } catch (const inviwo::IgnoreException& exception) {
        inviwo::util::log(
            exception.getContext(),
            "Incomplete network loading " + workspace + " due to " + exception.getMessage(),
            inviwo::LogLevel::Error);
        return 1;
    }

    inviwoApp.processFront();
    inviwoApp.getProcessorNetwork()->unlock();

    cmdparser.processCallbacks();  // run any command line callbacks from modules.

    if (cmdparser.getQuitApplicationAfterStartup()) {
        return 0;
    } else {
        return qtApp.exec();
    }
}
