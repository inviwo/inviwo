/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/localetools.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/networkdebugobserver.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/filesystem.h>

#include <modules/qtwidgets/propertylistwidget.h>  // for PropertyListWidget

#include <inviwo/sys/moduleloading.h>

#include <QApplication>
#include <QMainWindow>
#include <fmt/std.h>

int main(int argc, char** argv) {
    inviwo::util::configureCodePage();

    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);

    auto clogger = std::make_shared<inviwo::ConsoleLogger>();
    logger.registerLogger(clogger);

    // Must be set before constructing QApplication
    inviwo::utilqt::configureInviwoQtApp();

#ifdef IVW_DEBUG
    qInstallMessageHandler(&inviwo::utilqt::logQtMessages);
#endif

    QApplication qtApp{argc, argv};

    QMainWindow mainWindow{};
    mainWindow.setObjectName("InviwoMainWindow");
    mainWindow.setMinimumSize(800, 600);

    inviwo::InviwoApplication inviwoApp(argc, argv, "Inviwo-Qt");

    inviwo::utilqt::configureInviwoDefaultNames();
    inviwo::utilqt::configureFileSystemObserver(inviwoApp);
    inviwo::utilqt::configurePostEnqueueFront(inviwoApp);
    inviwo::utilqt::configureAssertionHandler(inviwoApp);
    inviwo::utilqt::setStyleSheetFile(":/stylesheets/inviwo.qss");
    inviwoApp.setUILocale(inviwo::utilqt::getCurrentStdLocale());

    inviwoApp.printApplicationInfo();
    inviwo::log::info("Qt Version {}", QT_VERSION_STR);

    inviwoApp.setProgressCallback([&logger](std::string_view m) {
        logger.log("InviwoApplication", inviwo::LogLevel::Info, inviwo::LogAudience::User, "", "",
                   0, m);
    });

    auto& cmdParser = inviwoApp.getCommandLineParser();

    // Initialize all modules
    inviwo::util::registerModules(inviwoApp.getModuleManager(),
                                  inviwoApp.getSystemSettings().moduleSearchPaths_.get(),
                                  cmdParser.getModuleSearchPaths());

    auto plw = new inviwo::PropertyListWidget(&mainWindow, &inviwoApp);
    mainWindow.addDockWidget(Qt::RightDockWidgetArea, plw);
    plw->setFloating(false);
    plw->hide();

    TCLAP::ValueArg<std::string> snapshotArg(
        "s", "snapshot",
        "Specify default name of each snapshot, or empty string for processor name.", false, "",
        "Snapshot default name: UPN=Use Processor name.");

    cmdParser.add(
        &snapshotArg,
        [&]() {
            auto path = cmdParser.getOutputPath();
            if (path.empty()) path = inviwo::filesystem::getPath(inviwo::PathType::Images);
            inviwo::util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path,
                                          snapshotArg.getValue());
        },
        1000);

    TCLAP::SwitchArg debugProcess("d", "debug",
                                  "Add debug logging for processor evaluation to the log");

    inviwo::NetworkDebugObserver obs;
    cmdParser.add(
        &debugProcess,
        [&]() {
            inviwoApp.getProcessorNetwork()->addObserver(&obs);
            inviwoApp.getProcessorNetworkEvaluator()->addObserver(&obs);
            inviwoApp.getProcessorNetwork()->forEachProcessor(
                [&](auto* p) { p->inviwo::ProcessorObservable::addObserver(&obs); });
        },
        200);

    TCLAP::SwitchArg fullscreenArg("f", "fullscreen", "Specify fullscreen if only one canvas");

    cmdParser.add(&fullscreenArg, [&]() {
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

    TCLAP::ValueArg<std::string> centralWidget(
        "", "central-widget",
        "Specify the identifier of the processor to use for the central widget.", false, "Canvas",
        "Processor Identifier");

    cmdParser.add(&centralWidget, [&]() {
        auto network = inviwoApp.getProcessorNetwork();
        if (auto processor = network->getProcessorByIdentifier(centralWidget.getValue())) {
            if (auto widget = dynamic_cast<QWidget*>(processor->getProcessorWidget())) {
                mainWindow.setCentralWidget(widget);
                widget->setWindowFlags(Qt::Widget);
                mainWindow.show();
            }
        }
    });

    TCLAP::MultiArg<std::string> exposeProperties(
        "", "property", "Specify a list of property paths to display in the property list widget",
        false, "Property path");

    cmdParser.add(&exposeProperties, [&]() {
        auto network = inviwoApp.getProcessorNetwork();
        for (auto& path : exposeProperties.getValue()) {
            auto [processorId, propertyPath] = inviwo::util::splitByFirst(path, '.');
            if (auto processor = network->getProcessorByIdentifier(processorId)) {
                if (propertyPath.empty()) {
                    plw->addProcessorProperties(processor);
                } else {
                    if (auto prop = processor->getPropertyByPath(propertyPath)) {
                        plw->addPropertyWidgets(prop);
                    }
                }
            }
        }
        plw->show();
    });

    // Do this after registerModules if some arguments were added
    cmdParser.parse();

    // Need to clear the network and (will delete processors and processorwidgets)
    // before QMainWindoes is deleted, otherwise it will delete all processorWidgets
    // before Processor can delete them.
    inviwo::util::OnScopeExit clearNetwork([&]() { inviwoApp.getProcessorNetwork()->clear(); });

    // Load workspace
    inviwoApp.getProcessorNetwork()->lock();

    const auto workspace = cmdParser.getLoadWorkspaceFromArg()
                               ? cmdParser.getWorkspacePath()
                               : inviwo::filesystem::getPath(inviwo::PathType::Workspaces, "/boron.inv");

    try {
        if (!workspace.empty()) {
            inviwoApp.getWorkspaceManager()->load(workspace, [&](inviwo::SourceContext) {
                try {
                    throw;
                } catch (const inviwo::IgnoreException& e) {
                    inviwo::log::error("Incomplete network loading {} due to:", workspace);
                    inviwo::log::exception(e);
                }
            });
        }
    } catch (const inviwo::AbortException& e) {
        inviwo::log::error("Unable to load network {} due to:", workspace);
        inviwo::log::exception(e);
        return 1;
    } catch (const inviwo::IgnoreException& e) {
        inviwo::log::error("Incomplete network loading {} due to:", workspace);
        inviwo::log::exception(e);
        return 1;
    }

    inviwoApp.processFront();
    inviwoApp.getProcessorNetwork()->unlock();

    cmdParser.processCallbacks();  // run any command line callbacks from modules.

    if (cmdParser.getQuitApplicationAfterStartup()) {
        return 0;
    } else {
        return qtApp.exec();
    }
}
