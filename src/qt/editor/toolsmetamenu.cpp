/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/toolsmetamenu.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/util/filesystem.h>

#include <inviwo/meta/creator.hpp>

#include <filesystem>
#include <sstream>

#include <warn/push>
#include <warn/ignore/all>
#include <QInputDialog>
#include <QMenu>
#include <warn/pop>

namespace inviwo {

void addInviwoMetaAction(QMenu* menu) {

    auto create = [](const std::string& name, auto func) {
        InviwoFileDialog saveFileDialog(nullptr, "Create " + name, "source");
        saveFileDialog.setFileMode(FileMode::AnyFile);
        saveFileDialog.setAcceptMode(AcceptMode::Save);
        saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

        if (saveFileDialog.exec()) {
            QString qpath = saveFileDialog.selectedFiles().at(0);
            const auto path = utilqt::fromQString(qpath);

            try {
                std::stringstream ss;
                meta::Creator creator(filesystem::findBasePath(), {}, {}, {true, false, false, ss});
                func(creator, std::filesystem::path{path});

                LogInfoCustom("Meta", ss.str());
            } catch (const std::runtime_error& e) {
                LogErrorCustom("Meta", e.what());
            }
        }
    };

    {
        auto action = menu->addAction("Add &File...");
        menu->connect(action, &QAction::triggered, [&]() {
            create("File", [](meta::Creator& creator, const std::filesystem::path& path) {
                creator.createFile(path);
            });
        });
    }
    {
        auto action = menu->addAction("Add &Processor...");
        menu->connect(action, &QAction::triggered, [&]() {
            create("Processor", [](meta::Creator& creator, const std::filesystem::path& path) {
                creator.createProcessor(path);
            });
        });
    }
    {
        auto action = menu->addAction("Add &Unit Test...");
        menu->connect(action, &QAction::triggered, [&]() {
            create("Test", [](meta::Creator& creator, const std::filesystem::path& path) {
                creator.createTest(path);
            });
        });
    }
    {
        auto action = menu->addAction("Add &Module...");
        menu->connect(action, &QAction::triggered, [&]() {
            create("Module", [](meta::Creator& creator, const std::filesystem::path& path) {
                bool ok;
                QString text = QInputDialog::getText(
                    nullptr, "Add Module", "Organization:", QLineEdit::Normal, "inviwo", &ok,
                    Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
                if (ok && !text.isEmpty()) {
                    creator.createModule(path, utilqt::fromQString(text));
                }
            });
        });
    }
}

}  // namespace inviwo
