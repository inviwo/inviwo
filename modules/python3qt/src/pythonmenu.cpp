/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>       // for InviwoModule
#include <inviwo/core/util/filedialogstate.h>      // for AcceptMode, AcceptMode::Save, FileMode
#include <inviwo/core/util/filesystem.h>           // for createDirectoryRecursively, getFileNam...
#include <inviwo/core/util/pathtype.h>             // for PathType, PathType::Settings
#include <modules/python3/python3module.h>         // for Python3Module
#include <modules/python3/pythonscript.h>          // for PythonScriptDisk
#include <modules/python3qt/pythoneditorwidget.h>  // for PythonEditorWidget
#include <modules/qtwidgets/inviwofiledialog.h>    // for InviwoFileDialog
#include <modules/qtwidgets/inviwoqtutils.h>       // for addMenu, fromQString, getApplicationMa...

#include <algorithm>    // for find_if
#include <fstream>      // for stringstream, basic_filebuf, basic_ifs...
#include <string>       // for basic_string, operator+
#include <type_traits>  // for remove_reference<>::type
#include <utility>      // for move

#include <QAction>           // for QAction
#include <QDesktopServices>  // for QDesktopServices
#include <QFileDialog>       // for QFileDialog, QFileDialog::DontConfirmO...
#include <QIcon>             // for QIcon
#include <QList>             // for QList
#include <QMainWindow>       // for QMainWindow
#include <QMenu>             // for QMenu
#include <QPoint>            // for QPoint, operator+
#include <QString>           // for operator+, QString
#include <QStringList>       // for QStringList
#include <QToolBar>          // for QToolBar
#include <QUrl>              // for QUrl, QUrl::TolerantMode
#include <Qt>                // for WA_DeleteOnClose
#include <QMenuBar>          // for QMenuBar
#include <fmt/core.h>        // for basic_string_view, arg
#include <fmt/ostream.h>     // for print
#include <fmt/std.h>

class QObject;

namespace inviwo {

PythonMenu::PythonMenu(InviwoModule* pymodule, InviwoApplication* app) {
    if (auto win = utilqt::getApplicationMainWindow()) {

        auto newEditor = [this, win, app]() {
            auto editor = std::make_unique<PythonEditorWidget>(win, app);
            editor->loadState();
            editor->setAttribute(Qt::WA_DeleteOnClose);
            if (!editors_.empty()) {
                auto newPos = editors_.back()->pos() + QPoint(40, 40);
                if (newPos.x() > 800) newPos.setX(350);
                if (newPos.y() > 800) newPos.setX(100);
                editor->move(newPos);
            }
            win->connect(editor.get(), &PythonEditorWidget::destroyed, [this](QObject* obj) {
                auto it = std::find_if(editors_.begin(), editors_.end(),
                                       [&](auto& elem) { return elem.get() == obj; });
                if (it != editors_.end()) {
                    it->release();
                    editors_.erase(it);
                }
            });

            editor->setVisible(true);
            editors_.push_back(std::move(editor));
        };

        toolbar_.reset(win->addToolBar("Python"));
        toolbar_->setObjectName("PythonToolBar");
        toolbar_->setMovable(false);
        toolbar_->setFloatable(false);
        win->connect(toolbar_.get(), &QToolBar::destroyed,
                     [this](QObject*) { toolbar_.release(); });
        auto newEditorOpen =
            toolbar_->addAction(QIcon(":/svgicons/python-mono.svg"), "Python Editor");
        win->connect(newEditorOpen, &QAction::triggered, newEditor);

        menu_.reset(utilqt::addMenu("&Python"));
        menu_->setParent(nullptr);

        auto pythonEditorOpen = menu_->addAction(QIcon(":/svgicons/python.svg"), "&Python Editor");
        win->connect(pythonEditorOpen, &QAction::triggered, newEditor);

        auto pyProperties = menu_->addAction("&List unexposed properties");
        win->connect(pyProperties, &QAction::triggered, [app]() {
            auto mod = app->getModuleByType<Python3Module>();
            PythonScriptDisk(mod->getPath() / "scripts" / "list_not_exposed_properties.py").run();
        });

        auto newPythonProcessor =
            menu_->addAction(QIcon(":/svgicons/processor-new.svg"), "&New Python Processor");
        toolbar_->addAction(newPythonProcessor);

        win->connect(newPythonProcessor, &QAction::triggered, [pymodule, app]() {
            InviwoFileDialog saveFileDialog(nullptr, "Create Python Processor", "PythonProcessor");
            saveFileDialog.setFileMode(FileMode::AnyFile);
            saveFileDialog.setAcceptMode(AcceptMode::Save);
            saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);
            saveFileDialog.addExtension("py", "Python file");
            const auto dir = app->getPath(PathType::Settings) / "python_processors";
            std::filesystem::create_directories(dir);
            saveFileDialog.setCurrentDirectory(dir);

            if (saveFileDialog.exec()) {
                QString qpath = saveFileDialog.selectedFiles().at(0);
                const auto path = utilqt::toPath(qpath);

                const auto templatePath = pymodule->getPath() / "templates/templateprocessor.py";

                auto ifs = std::ifstream(templatePath);
                std::stringstream ss;
                ss << ifs.rdbuf();
                const auto script = std::move(ss).str();

                const auto name = path.stem().generic_string();

                auto ofs = std::ofstream(path);
                fmt::print(ofs, fmt::runtime(script), fmt::arg("name", name));

                QDesktopServices::openUrl(QUrl("file:///" + qpath, QUrl::TolerantMode));
            }
        });
    }
}

PythonMenu::~PythonMenu() {
    if (auto win = utilqt::getApplicationMainWindow()) {
        if (menu_) {
            win->menuBar()->removeAction(menu_->menuAction());
        }
    }
}

}  // namespace inviwo
