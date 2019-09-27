/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <modules/qtwidgets/inviwofiledialog.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/filesystem.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QLayout>
#include <QDesktopServices>
#include <QUrl>
#include <warn/pop>

namespace inviwo {

PythonMenu::PythonMenu(InviwoModule* pymodule, InviwoApplication* app) {
    if (auto win = utilqt::getApplicationMainWindow()) {

        menu_.reset(utilqt::addMenu("&Python"));
        win->connect(menu_.get(), &QMenu::destroyed, [this](QObject*) { menu_.release(); });

        auto pythonEditorOpen = menu_->addAction(QIcon(":/icons/python.png"), "&Python Editor");

        win->connect(pythonEditorOpen, &QAction::triggered, [this, win, app]() {
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
        });

        auto pyProperties = menu_->addAction("&List unexposed properties");
        win->connect(pyProperties, &QAction::triggered, [app]() {
            auto mod = app->getModuleByType<Python3Module>();
            PythonScriptDisk(mod->getPath() + "/scripts/list_not_exposed_properties.py").run();
        });

        auto newPythonProcessor =
            menu_->addAction(QIcon(":/svgicons/processor-new.svg"), "&New Python Processor");
        win->connect(newPythonProcessor, &QAction::triggered, [pymodule, app]() {
            InviwoFileDialog saveFileDialog(nullptr, "Create Python Processor", "PythonProcessor");
            saveFileDialog.setFileMode(FileMode::AnyFile);
            saveFileDialog.setAcceptMode(AcceptMode::Save);
            saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);
            saveFileDialog.addExtension("py", "Python file");
            const auto dir = app->getPath(PathType::Settings) + "/python_processors";
            filesystem::createDirectoryRecursively(dir);
            saveFileDialog.setCurrentDirectory(dir);

            if (saveFileDialog.exec()) {
                QString qpath = saveFileDialog.selectedFiles().at(0);
                const auto path = utilqt::fromQString(qpath);

                const auto templatePath = pymodule->getPath() + "/templates/templateprocessor.py";

                auto ifs = filesystem::ifstream(templatePath);
                std::stringstream ss;
                ss << ifs.rdbuf();
                const auto script = std::move(ss).str();

                const auto name = filesystem::getFileNameWithoutExtension(path);

                auto ofs = filesystem::ofstream(path);
                fmt::print(ofs, script, fmt::arg("name", name));

                QDesktopServices::openUrl(QUrl("file:///" + qpath, QUrl::TolerantMode));
            }
        });
    }
}

PythonMenu::~PythonMenu() = default;

}  // namespace inviwo
