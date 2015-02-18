/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "inviwosplashscreen.h"

#include <QApplication>
#include <QPainter>
#include <QSplashScreen>
#include <QTextStream>

#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>

namespace inviwo {

InviwoSplashScreen::InviwoSplashScreen()
    : QSplashScreen(dynamic_cast<InviwoApplicationQt*>(inviwo::InviwoApplicationQt::getPtr())->getMainWindow(),
                    QPixmap(":/images/splashscreen.png") /*,Qt::WindowStaysOnTopHint*/)
{
    const CommandLineParser* cmdparser = inviwo::InviwoApplicationQt::getPtr()->getCommandLineParser();
    showSplashScreen_ = cmdparser->getShowSplashScreen();
}

InviwoSplashScreen::~InviwoSplashScreen() {}

void InviwoSplashScreen::show() {
    if (showSplashScreen_)
        QSplashScreen::show();
}

void InviwoSplashScreen::drawContents(QPainter* painter) {
    QSplashScreen::drawContents(painter);
    QString versionLabel;
    QTextStream labelStream(&versionLabel);
    labelStream << "Version " << QString::fromStdString(IVW_VERSION);
    painter->drawText(13, 265, versionLabel);
}

void InviwoSplashScreen::showMessage(std::string message) {
    // show message and add whitespace to match layout
    if (showSplashScreen_)
        QSplashScreen::showMessage(QString::fromStdString("   "+message), Qt::AlignLeft|Qt::AlignBottom, Qt::white);
}

void InviwoSplashScreen::finish(QWidget* mainWindow) {
    if (showSplashScreen_)
        QSplashScreen::finish(mainWindow);
}

} // namespace