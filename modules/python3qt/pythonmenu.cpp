/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

namespace inviwo {

PythonMenu::PythonMenu(InviwoApplication* app) {
    if (auto win = dynamic_cast<InviwoApplicationQt*>(app)->getMainWindow()) {
        if (auto ivwwin = dynamic_cast<InviwoMainWindow*>(win)) {
            QMenu* menu = win->menuBar()->addMenu("Python");
            QAction* pythonEditorOpen =
                menu->addAction(QIcon(":/icons/python.png"), "&Python Editor");
            editor_ = new PythonEditorWidget(ivwwin, app);
            win->connect(pythonEditorOpen, SIGNAL(triggered(bool)), editor_, SLOT(show(void)));
        }
    }
}

PythonMenu::~PythonMenu() {
}

PythonEditorWidget* PythonMenu::getEditor() const {
    return editor_;
}

} // namespace

