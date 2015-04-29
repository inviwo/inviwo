/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#ifdef IVW_DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif
#endif

#include <locale>

#include <QFile>
#include <QLocale>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>
#include "inviwomainwindow.h"
#include "inviwosplashscreen.h"
#include <moduleregistration.h>

int main(int argc, char** argv) {
    std::string basePath = inviwo::filesystem::findBasePath();
    
    inviwo::LogCentral::init();
    inviwo::LogCentral::getPtr()->registerLogger(new inviwo::FileLogger(basePath));
    inviwo::InviwoApplicationQt inviwoApp("Inviwo v"+IVW_VERSION, basePath, argc, argv);
    inviwoApp.setWindowIcon(QIcon(":/icons/inviwo_light.png"));
    inviwoApp.setAttribute(Qt::AA_NativeWindows);
    QFile styleSheetFile(":/stylesheets/inviwo.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    inviwoApp.setStyleSheet(styleSheet);

    // set output locale to current locale for printf and streams
    try
    {
        // need to change locale given by Qt from underscore to hyphenated ("sv_SE" to "sv-SE")
        // although std::locale should only accept locales with underscore, e.g. "sv_SE"
        std::string localeName(QLocale::system().name().replace('_', '-').toStdString());

        std::locale loc(localeName);
        std::locale loc1;
        std::cout << "The initial locale is: " << loc1.name() << std::endl;
        // setting both, C and C++ locale
        std::locale loc2 = std::locale::global(loc);
        std::locale loc3;
        std::cout << "The current locale is: " << loc3.name() << std::endl;
        std::cout << "The previous locale was: " << loc2.name() << std::endl;
    }
    catch (std::exception& e) {
        LogErrorCustom("main", "Error setting locale: " << e.what());
    }

    inviwo::InviwoMainWindow mainWin;
    // setup core application
    inviwoApp.setMainWindow(&mainWin);
    // initialize and show splash screen
    inviwo::InviwoSplashScreen splashScreen;
    bool showSplashScreen = inviwoApp.getCommandLineParser()->getShowSplashScreen();
    if (showSplashScreen) {
        splashScreen.show();
        splashScreen.showMessage("Loading application...");
    }
    styleSheetFile.close();
    //Initialize application and register modules
    splashScreen.showMessage("Initializing modules...");
    inviwoApp.initialize(&inviwo::registerAllModules);
    // setup main window
    mainWin.initialize();
    splashScreen.showMessage("Loading workspace...");
    mainWin.initializeWorkspace();
    mainWin.showWindow();
    // open last workspace
    mainWin.openLastWorkspace();
    splashScreen.finish(&mainWin);
#if defined(REG_INVIWOUNITTESTSMODULE) && defined(IVW_RUN_UNITTEST_ON_STARTUP)
    inviwo::UnitTestsModule::runAllTests();
#endif

    // process last arguments
    if (mainWin.processCommandLineArgs())
        return inviwoApp.exec();
    else{
        mainWin.exitInviwo();
        return  0;
    }
}
