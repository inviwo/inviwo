/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/qtapptools.h>
#include <inviwo/qt/applicationbase/qtlocale.h>
#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/util/filelogger.h>
#include "inviwosplashscreen.h"
#include <inviwo/core/moduleregistration.h>

#include <sstream>
#include <algorithm>

#include <QMessageBox>
#include <QApplication>

#ifndef INVIWO_ALL_DYN_LINK
#include <inviwopy/inviwopy.h>
#endif

int main(int argc, char** argv) {
    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);
    auto logCounter = std::make_shared<inviwo::LogErrorCounter>();
    logger.registerLogger(logCounter);
#ifdef __linux__
    /*
     * Suppress warning "QApplication: invalid style override passed, ignoring it." when starting
     * Inviwo on Linux. See
     * https://forum.qt.io/topic/75398/qt-5-8-0-qapplication-invalid-style-override-passed-ignoring-it/2
     */
    qputenv("QT_STYLE_OVERRIDE", "");
#endif
    // Must be set before constructing QApplication
    inviwo::utilqt::configureInviwoQtApp();

#ifdef IVW_DEBUG
    qInstallMessageHandler(&inviwo::utilqt::logQtMessages);
#endif

    QApplication qtApp{argc, argv};
    inviwo::InviwoApplication inviwoApp(argc, argv, "Inviwo");

    inviwo::utilqt::configureInviwoDefaultNames();
    inviwo::utilqt::configureFileSystemObserver(inviwoApp);
    inviwo::utilqt::configurePostEnqueueFront(inviwoApp);
    inviwo::utilqt::setStyleSheetFile(":/stylesheets/inviwo.qss");
    inviwoApp.setUILocale(inviwo::utilqt::getCurrentStdLocale());

    auto& clp = inviwoApp.getCommandLineParser();

    inviwo::InviwoMainWindow mainWin(&inviwoApp);
    inviwoApp.printApplicationInfo();
    LogInfoCustom("InviwoInfo", "Qt Version " << QT_VERSION_STR);

    // initialize and show splash screen
    inviwo::InviwoSplashScreen splashScreen(clp.getShowSplashScreen());
    inviwoApp.setProgressCallback([&splashScreen](std::string s) { splashScreen.showMessage(s); });

    splashScreen.show();
    splashScreen.showMessage("Loading application...");

    // Initialize application and register modules
    splashScreen.showMessage("Initializing modules...");
    
    inviwoApp.registerModules(inviwo::getModuleList());

#ifndef INVIWO_ALL_DYN_LINK
    inviwo::initInviwoPy();
#endif

    qtApp.processEvents();

    // Do this after registerModules if some arguments were added
    clp.parse(inviwo::CommandLineParser::Mode::Normal);

    qtApp.processEvents();  // Update GUI
    splashScreen.showMessage("Loading workspace...");
    qtApp.processEvents();
    mainWin.showWindow();
    qtApp.processEvents();  // Make sure the gui is done loading before loading workspace

    mainWin.openLastWorkspace(clp.getWorkspacePath());  // open last workspace
    inviwoApp.setProgressCallback(std::function<void(std::string)>{});
    splashScreen.finish(&mainWin);

    qtApp.processEvents();
    clp.processCallbacks();  // run any command line callbacks from modules.
    qtApp.processEvents();

    inviwo::util::OnScopeExit clearNetwork([&]() { inviwoApp.getProcessorNetwork()->clear(); });

    if (auto numErrors = logCounter->getWarnCount()) {
        LogWarnCustom("inviwo.cpp", numErrors << " warnings generated during startup");
    }
    if (auto numErrors = logCounter->getErrorCount()) {
        LogErrorCustom("inviwo.cpp", numErrors << " errors generated during startup");
    }
    logCounter.reset();

    // process last arguments
    if (clp.getQuitApplicationAfterStartup()) {
        mainWin.exitInviwo(false);
        return 0;
    }

    inviwo::util::setThreadDescription("Inviwo Main");

    while (true) {
        try {
            return qtApp.exec();
        } catch (const inviwo::Exception& e) {
            {
                std::stringstream ss;
                ss << e.getMessage() << "\n";
                if (!e.getStack().empty()) {
                    ss << "\nStack Trace:\n";
                    e.getStack(ss);
                }
                inviwo::util::log(e.getContext(), ss.str(), inviwo::LogLevel::Error);
            }
            {
                std::stringstream ss;
                e.getFullMessage(ss, 10);
                ss << "\nApplication state might be corrupted, be warned.";
                auto res = QMessageBox::critical(
                    &mainWin, "Fatal Error", QString::fromStdString(ss.str()),
                    QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
                if (res == QMessageBox::Close) {
                    mainWin.askToSaveWorkspaceChanges();
                    return 1;
                }
            }

        } catch (const std::exception& e) {
            LogErrorCustom("Inviwo", e.what());
            std::stringstream ss;
            ss << e.what();
            ss << "\nApplication state might be corrupted, be warned.";
            auto res =
                QMessageBox::critical(&mainWin, "Fatal Error", QString::fromStdString(ss.str()),
                                      QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
            if (res == QMessageBox::Close) {
                mainWin.askToSaveWorkspaceChanges();
                return 1;
            }
        } catch (...) {
            LogErrorCustom("Inviwo", "Uncaught exception, terminating");
            QMessageBox::critical(nullptr, "Fatal Error", "Uncaught exception, terminating");
            return 1;
        }
    }
}
