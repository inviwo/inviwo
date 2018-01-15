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

#include <modules/demoqt/demoqtmodule.h>
#include <modules/demoqt/demonavigatordockwidgetqt.h>

#include <modules/demo/demomodule.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

namespace inviwo {

DemoQtModule::DemoQtModule(InviwoApplication* app) : InviwoModule(app, "DemoQt") {
    if (auto win = utilqt::getApplicationMainWindow()) {
        QString demoMenuName("Demo");
        QMenu* menu = nullptr;

        // Find view menu
        auto menus = win->menuBar()->findChildren<QMenu*>();
        auto viewMenuIt = std::find_if(menus.begin(), menus.end(), [](auto& m) {
            return m->title().compare(QObject::tr("&View"), Qt::CaseInsensitive) == 0;
        });

        if (viewMenuIt != menus.end()) {
            // Add to view menu if found
            menu = (*viewMenuIt);
        } else {
            // Add new menu if not found
            // To be removed when module is destroyed
            menu_ = std::make_unique<QMenu>(demoMenuName);
            win->menuBar()->addMenu(menu_.get());
            menu = menu_.get();

            // Release pointer if destroyed by Qt before module is destroyed
            QObject::connect(menu_.get(), &QObject::destroyed,
                             [&](QObject* obj) { menu_.release(); });
        }
        auto& controller =
            app->getModuleByType<DemoModule>()->getDemoManager().getDemoController();
        navigator_ = std::make_unique<demo::DemoNavigatorDockWidgetQt>(controller,
                                                                    "Demo Navigator", win);
        menu->addAction(navigator_->toggleViewAction());
        navigator_->hide();
        navigator_->loadState();
        // Release pointer if destroyed by Qt before module is destroyed
        QObject::connect(navigator_.get(), &QObject::destroyed,
                         [this](QObject* obj) { navigator_.release(); });
    }
}

DemoQtModule::~DemoQtModule() = default;

}  // namespace inviwo
