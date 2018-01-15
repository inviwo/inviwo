/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/demoqt/demonavigatordockwidgetqt.h>
#include <modules/demoqt/demonavigatorqt.h>

#include <modules/demo/democontroller.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QSettings>
#include <QToolBar>
#include <QMainWindow>
#include <warn/pop>

namespace inviwo {

namespace demo {

DemoNavigatorDockWidgetQt::DemoNavigatorDockWidgetQt(DemoController& controller,
                                                         const std::string& widgetName,
                                                         QWidget* parent)
    : InviwoDockWidget(QString(widgetName.c_str()), parent, "DemoNavigatorWidget")
    , controller_(controller) {

    setFloating(true);
    setSticky(false);
    resize(QSize(300, 100)); // default size
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setWindowIcon(
        QIcon(":/demo/icons/arrow_next_player_previous_recording_right_icon_128.png"));

    addObservation(&controller_);
}

DemoNavigatorDockWidgetQt::~DemoNavigatorDockWidgetQt() = default;

void DemoNavigatorDockWidgetQt::onStateChanged(DemoController* controller) {

}

}  // namespace demo

}  // namespace inviwo
