/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include <modules/openglqt/shaderwidget.h>

#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/qtwidgets/properties/syntaxhighlighter.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/filesystem.h>

#include <modules/qtwidgets/codeedit.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTextEdit>
#include <QDialog>
#include <QToolBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QGridLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QFont>
#include <warn/pop>

namespace inviwo {

ShaderWidget::ShaderWidget(const ShaderObject* obj, QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(obj->getFileName()), parent, "ShaderEditorWidget")
    , obj_(obj) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(QSize(500, 700));  // default size
    setFloating(true);
    setSticky(false);

    QMainWindow* mainWindow = new QMainWindow();
    mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow);

    auto shadercode = new CodeEdit{GLSL};
    shadercode->setObjectName("shaderwidgetcode");
    shadercode->setPlainText(utilqt::toQString(obj->print(false, false)));

    auto save = toolBar->addAction(QIcon(":/icons/save.png"), tr("&Save shader"));
    save->setShortcut(QKeySequence::Save);
    save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    mainWindow->addAction(save);
    connect(save, &QAction::triggered, [=]() {
        if (auto fr = dynamic_cast<const FileShaderResource*>(obj->getResource().get())) {
            auto file = filesystem::ofstream(fr->file());
            file << utilqt::fromQString(shadercode->toPlainText());
            file.close();
        } else if (auto sr = dynamic_cast<const StringShaderResource*>(obj->getResource().get())) {
            // get the non-const version from the manager.
            auto res = ShaderManager::getPtr()->getShaderResource(sr->key());
            if (auto editable = dynamic_cast<StringShaderResource*>(res.get())) {
                editable->setSource(utilqt::fromQString(shadercode->toPlainText()));
            }
        }
    });

    QPixmap enabled(":/icons/precompiled.png");
    QPixmap disabled(":/icons/precompiled-disabled.png");
    QIcon preicon;
    preicon.addPixmap(enabled, QIcon::Normal, QIcon::Off);
    preicon.addPixmap(disabled, QIcon::Normal, QIcon::On);

    auto preprocess = toolBar->addAction(preicon, "Show Preprocessed");
    preprocess->setChecked(false);
    preprocess->setCheckable(true);

    auto updateState = [=]() {
        const auto code = obj_->print(false, preprocess->isChecked());
        shadercode->setPlainText(utilqt::toQString(code));
        if (preprocess->isChecked()) {
            const auto lines = std::count(code.begin(), code.end(), '\n') + 1;
            std::string::size_type width = 0;
            for (size_t l = 0; l < static_cast<size_t>(lines); ++l) {
                auto info = obj_->resolveLine(l);
                auto pos = info.first.find_last_of('/');
                width = std::max(width, info.first.size() - (pos + 1));  // note string::npos+1==0
            }
            const auto numberSize = std::to_string(lines).size();
            shadercode->setLineAnnotation([this, width, numberSize](int line) {
                const auto info = obj_->resolveLine(line - 1);
                const auto pos = info.first.find_last_of('/');
                const auto file = info.first.substr(pos + 1);
                std::stringstream out;
                out << std::left << std::setw(width + 1u) << file << std::right
                    << std::setw(numberSize) << info.second;
                return out.str();
            });
            shadercode->setAnnotationSpace(
                [width, numberSize](int) { return static_cast<int>(width + 1 + numberSize); });
        } else {
            shadercode->setLineAnnotation([](int line) { return std::to_string(line); });
            shadercode->setAnnotationSpace([](int maxDigits) { return maxDigits; });
        }

        shadercode->setReadOnly(preprocess->isChecked());
        save->setEnabled(!preprocess->isChecked());
        preprocess->setText(preprocess->isChecked() ? "Hide Preprocessed" : "Show Preprocessed");
    };
    connect(preprocess, &QAction::triggered, this, updateState);
    updateState();

    mainWindow->setCentralWidget(shadercode);

    loadState();
}

ShaderWidget::~ShaderWidget() = default;

}  // namespace inviwo
