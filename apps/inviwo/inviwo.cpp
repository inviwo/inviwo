/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/common/modulemanager.h>

#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/localetools.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include "inviwosplashscreen.h"
#include <inviwo/sys/moduleloading.h>
#include <inviwo/py/pythonhelper.h>

#include <sstream>
#include <algorithm>
#include <array>

#include <QMessageBox>
#include <QApplication>

int main(int argc, char** argv) {
    inviwo::util::configureCodePage();

    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);
    auto logCounter = std::make_shared<inviwo::LogErrorCounter>();
    logger.registerLogger(logCounter);

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
    inviwo::utilqt::configurePalette();
    inviwoApp.setUILocale(inviwo::utilqt::getCurrentStdLocale());

    auto& clp = inviwoApp.getCommandLineParser();

    inviwo::InviwoMainWindow mainWin(&inviwoApp);
    inviwoApp.printApplicationInfo();
    inviwo::log::user::info("Qt Version {}", QT_VERSION_STR);

    // initialize and show splash screen
    inviwo::InviwoSplashScreen splashScreen(clp.getShowSplashScreen());
    inviwoApp.setProgressCallback([&](std::string_view s) { splashScreen.showMessage(s); });

    inviwo::initializePythonModules();

    splashScreen.show();
    splashScreen.showMessage("Loading application...");
    qtApp.processEvents();
    // Initialize application and register modules
    splashScreen.showMessage("Initializing modules...");

    // Remove GLFW module register since we will use Qt for the OpenGL context
    auto filter = [](const inviwo::ModuleContainer& m) { return m.identifier() == "glfw"; };
    inviwo::util::registerModulesFiltered(inviwoApp.getModuleManager(), filter,
                                          inviwoApp.getSystemSettings().moduleSearchPaths_.get(),
                                          clp.getModuleSearchPaths());

    qtApp.processEvents();

    // Do this after registerModules if some arguments were added
    clp.parse();

    qtApp.processEvents();  // Update GUI
    splashScreen.showMessage("Loading workspace...");
    qtApp.processEvents();
    mainWin.showWindow();
    qtApp.processEvents();  // Make sure the gui is done loading before loading workspace

    mainWin.openLastWorkspace(clp.getWorkspacePath());  // open last workspace
    inviwoApp.setProgressCallback(nullptr);
    splashScreen.finish(&mainWin);

    qtApp.processEvents();
    clp.processCallbacks();  // run any command line callbacks from modules.
    qtApp.processEvents();

    inviwo::util::OnScopeExit clearNetwork([&]() { inviwoApp.getProcessorNetwork()->clear(); });

    if (auto numErrors = logCounter->getWarnCount()) {
        inviwo::log::user::warn("{} warnings generated during startup", numErrors);
    }
    if (auto numErrors = logCounter->getErrorCount()) {
        inviwo::log::user::warn("{} errors generated during startup", numErrors);
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
            inviwo::log::user::exception(e);
            const auto message = fmt::format("{}\nApplication state might be corrupted, be warned.",
                                             e.getFullMessage(10));
            auto res =
                QMessageBox::critical(&mainWin, "Fatal Error", inviwo::utilqt::str(message),
                                      QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
            if (res == QMessageBox::Close) {
                mainWin.askToSaveWorkspaceChanges();
                return 1;
            }

        } catch (const std::exception& e) {
            inviwo::log::user::exception(e);
            const auto message =
                fmt::format("{}\nApplication state might be corrupted, be warned.", e.what());
            auto res =
                QMessageBox::critical(&mainWin, "Fatal Error", inviwo::utilqt::str(message),
                                      QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
            if (res == QMessageBox::Close) {
                mainWin.askToSaveWorkspaceChanges();
                return 1;
            }
        } catch (...) {
            inviwo::log::user::exception("Uncaught exception, terminating");
            QMessageBox::critical(nullptr, "Fatal Error", "Uncaught exception, terminating");
            return 1;
        }
    }
}
