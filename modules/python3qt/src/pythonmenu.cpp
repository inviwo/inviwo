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

#include <modules/python3qt/pythonmenu.h>

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>       // for InviwoModule
#include <inviwo/core/util/filedialogstate.h>      // for AcceptMode, AcceptMode::Save, FileMode
#include <inviwo/core/util/filesystem.h>           // for createDirectoryRecursively, getFileNam...
#include <inviwo/core/util/moduleutils.h>
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

PythonMenu::PythonMenu(const std::filesystem::path& modulePath, InviwoApplication* app,
                       QMainWindow* win)
    : app_{app}
    , win_{win}
    , editors_{}
    , toolbar_{win_->addToolBar("Python")}
    , menu_{utilqt::addMenu("&Python")}
    , scriptMenu_{util::getModuleByTypeOrThrow<Python3Module>(app).getWorkspaceScripts(),
                  menu_.get(), app, win} {

    toolbar_->setObjectName("PythonToolBar");
    toolbar_->setMovable(false);
    toolbar_->setFloatable(false);

    auto newEditorOpen = toolbar_->addAction(QIcon(":/svgicons/python-mono.svg"), "Python Editor");
    QObject::connect(newEditorOpen, &QAction::triggered, [this]() { newEditor(); });

    menu_->setObjectName("PythonMenu");

    auto pythonEditorOpen = menu_->addAction(QIcon(":/svgicons/python.svg"), "&Python Editor");
    QObject::connect(pythonEditorOpen, &QAction::triggered, [this]() { newEditor(); });

    auto newPythonProcessor =
        menu_->addAction(QIcon(":/svgicons/processor-new.svg"), "&New Python Processor");
    toolbar_->addAction(newPythonProcessor);

    QObject::connect(newPythonProcessor, &QAction::triggered, [modulePath]() {
        try {
            InviwoFileDialog saveFileDialog(nullptr, "Create Python Processor", "PythonProcessor");
            saveFileDialog.setFileMode(FileMode::AnyFile);
            saveFileDialog.setAcceptMode(AcceptMode::Save);
            saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);
            saveFileDialog.addExtension("py", "Python file");
            const auto dir = filesystem::getPath(PathType::Settings) / "python_processors";
            std::filesystem::create_directories(dir);
            saveFileDialog.setCurrentDirectory(dir);

            if (saveFileDialog.exec()) {
                QString qpath = saveFileDialog.selectedFiles().at(0);
                const auto path = utilqt::toPath(qpath);

                const auto templatePath = modulePath / "templates/templateprocessor.py";

                auto ifs = std::ifstream(templatePath);
                std::stringstream ss;
                ss << ifs.rdbuf();
                const auto script = std::move(ss).str();

                const auto name = path.stem().generic_string();

                auto ofs = std::ofstream(path);
                fmt::print(ofs, fmt::runtime(script), fmt::arg("name", name));

                QDesktopServices::openUrl(QUrl("file:///" + qpath, QUrl::TolerantMode));
            }
        } catch (const std::exception& e) {
            log::exception(e);
        }
    });

    auto pyProperties = menu_->addAction("&List unexposed properties");
    QObject::connect(pyProperties, &QAction::triggered, [app]() {
        auto& mod = util::getModuleByTypeOrThrow<Python3Module>(app);
        auto script =
            PythonScript::fromFile(mod.getPath() / "scripts" / "list_not_exposed_properties.py");
        script.run();
    });
}

PythonEditorWidget* PythonMenu::newEditor() {
    auto editor = util::make_qptr<PythonEditorWidget>(win_, app_);
    editor->loadState();
    editor->restore();

    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setVisible(true);
    editors_.push_back(std::move(editor));

    return editors_.back().get();
}

PythonMenu::~PythonMenu() {
    if (menu_) {
        win_->menuBar()->removeAction(menu_->menuAction());
    }
    if (toolbar_) {
        win_->removeToolBar(toolbar_.get());
    }
}

}  // namespace inviwo
