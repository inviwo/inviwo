/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#include <modules/python3qt/pythonmenu.h>
#include <modules/python3qt/pythoneditorwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QLayout>
#include <warn/pop>

namespace inviwo {

PythonMenu::PythonMenu(InviwoApplication* app) {
    if (auto win = utilqt::getApplicationMainWindow()) {
        menu_ = utilqt::addMenu("&Python");
        auto pythonEditorOpen = menu_->addAction(QIcon(":/icons/python.png"), "&Python Editor");
        editor_ = new PythonEditorWidget(win, app);
        editor_->setVisible(false);
        editor_->loadState();
        win->connect(pythonEditorOpen, &QAction::triggered, editor_, &PythonEditorWidget::show);

        auto pyPropertoes = menu_->addAction("&List unexposed properties");
        win->connect(pyPropertoes, &QAction::triggered, [app]() {
            auto mod = app->getModuleByType<Python3Module>();
            PythonScriptDisk(mod->getPath() + "/scripts/list_not_exposed_properties.py").run();
        });
    }
}

PythonMenu::~PythonMenu() {
    if (auto win = utilqt::getApplicationMainWindow()) {
        // Delete menu_ and editor_ since the MainWindow is parent and will
        // not delete menu_ until after module has been deinitialized.
        // Destructors will remove the created widgets, actions and signals
        delete menu_;
        delete editor_;
    }
}

PythonEditorWidget* PythonMenu::getEditor() const { return editor_; }

}  // namespace inviwo
