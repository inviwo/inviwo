/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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
#include <QToolBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSignalBlocker>
#include <warn/pop>

namespace inviwo {

ShaderWidget::ShaderWidget(const ShaderObject* obj, QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(obj->getFileName()) + "[*]", parent, "ShaderEditorWidget")
    , obj_(obj)
    , fileObserver_(this, "Shader Editor", getFileName(obj)) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(50, 70)));  // default size
    setFloating(true);
    setSticky(false);

    QMainWindow* mainWindow = new QMainWindow();
    mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow);

    shadercode_ = new CodeEdit{GLSL};
    shadercode_->setObjectName("shaderwidgetcode");
    shadercode_->setPlainText(utilqt::toQString(obj->print(false, false)));

    auto save = toolBar->addAction(QIcon(":/svgicons/save.svg"), tr("&Save Shader"));
    save->setShortcut(QKeySequence::Save);
    save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    mainWindow->addAction(save);

    connect(save, &QAction::triggered, this, &ShaderWidget::save);

    QPixmap enabled(":/svgicons/precompiled-enabled.svg");
    QPixmap disabled(":/svgicons/precompiled-disabled.svg");
    QIcon preicon;
    preicon.addPixmap(enabled, QIcon::Normal, QIcon::Off);
    preicon.addPixmap(disabled, QIcon::Normal, QIcon::On);

    auto preprocess = toolBar->addAction(preicon, "Show Preprocessed Shader");
    preprocess->setChecked(false);
    preprocess->setCheckable(true);

    auto revert = toolBar->addAction(QIcon(":/svgicons/revert.svg"), tr("Revert"));
    revert->setToolTip("Revert changes");
    revert->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    revert->setEnabled(false);
    QObject::connect(shadercode_, &QPlainTextEdit::modificationChanged, revert,
                     &QAction::setEnabled);

    toolBar->addSeparator();
    auto undo = toolBar->addAction(QIcon(":/svgicons/undo.svg"), "&Undo");
    undo->setShortcut(QKeySequence::Undo);
    undo->setEnabled(false);
    QObject::connect(undo, &QAction::triggered, this, [this]() { shadercode_->undo(); });
    QObject::connect(shadercode_, &QPlainTextEdit::undoAvailable, undo, &QAction::setEnabled);

    auto redo = toolBar->addAction(QIcon(":/svgicons/redo.svg"), "&Redo");
    redo->setShortcut(QKeySequence::Redo);
    redo->setEnabled(false);
    QObject::connect(redo, &QAction::triggered, this, [this]() { shadercode_->redo(); });
    QObject::connect(shadercode_, &QPlainTextEdit::redoAvailable, redo, &QAction::setEnabled);

    auto updateState = [=]() {
        const bool checked = preprocess->isChecked();
        const auto code = obj_->print(false, checked);
        shadercode_->setPlainText(utilqt::toQString(code));
        if (checked) {
            const auto lines = std::count(code.begin(), code.end(), '\n') + 1;
            std::string::size_type width = 0;
            for (size_t l = 0; l < static_cast<size_t>(lines); ++l) {
                auto info = obj_->resolveLine(l);
                auto pos = info.first.find_last_of('/');
                width = std::max(width, info.first.size() - (pos + 1));  // note string::npos+1==0
            }
            const auto numberSize = std::to_string(lines).size();
            shadercode_->setLineAnnotation([this, width, numberSize](int line) {
                const auto info = obj_->resolveLine(line - 1);
                const auto pos = info.first.find_last_of('/');
                const auto file = info.first.substr(pos + 1);
                std::stringstream out;
                out << std::left << std::setw(width + 1u) << file << std::right
                    << std::setw(numberSize) << info.second;
                return out.str();
            });
            shadercode_->setAnnotationSpace(
                [width, numberSize](int) { return static_cast<int>(width + 1 + numberSize); });
        } else {
            shadercode_->setLineAnnotation([](int line) { return std::to_string(line); });
            shadercode_->setAnnotationSpace([](int maxDigits) { return maxDigits; });
        }

        shadercode_->setReadOnly(checked);
        save->setEnabled(!checked);
        preprocess->setText(checked ? "Show Plain Shader Only" : "Show Preprocessed Shader");
    };
    QObject::connect(preprocess, &QAction::toggled, this, [=](bool checked) {
        if (checked && shadercode_->document()->isModified()) {
            QMessageBox msgBox(
                QMessageBox::Question, "Shader Editor", "Do you want to save unsaved changes?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

            int retval = msgBox.exec();
            if (retval == static_cast<int>(QMessageBox::Save)) {
                this->save();
            } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
                QSignalBlocker block(preprocess);
                preprocess->setChecked(false);
                return;
            }
        }
        updateState();
    });
    QObject::connect(revert, &QAction::triggered, this, updateState);
    QObject::connect(shadercode_, &QPlainTextEdit::modificationChanged, this,
                     &QDockWidget::setWindowModified);
    shadercode_->installEventFilter(&fileObserver_);

    fileObserver_.setReloadFileCallback(updateState);
    fileObserver_.setModifiedCallback([this](bool m) { shadercode_->document()->setModified(m); });

    mainWindow->setCentralWidget(shadercode_);

    updateState();
    loadState();
}

ShaderWidget::~ShaderWidget() = default;

void ShaderWidget::closeEvent(QCloseEvent* event) {
    if (shadercode_->document()->isModified()) {
        QMessageBox msgBox(QMessageBox::Question, "Shader Editor",
                           "Do you want to save unsaved changes?",
                           QMessageBox::Save | QMessageBox::Discard, this);

        int retval = msgBox.exec();
        if (retval == static_cast<int>(QMessageBox::Save)) {
            save();
        } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
            return;
        }
    }
    InviwoDockWidget::closeEvent(event);
}

void ShaderWidget::save() {
    if (auto fr = dynamic_cast<const FileShaderResource*>(obj_->getResource().get())) {
        fileObserver_.ignoreNextUpdate();
        auto file = filesystem::ofstream(fr->file());
        file << utilqt::fromQString(shadercode_->toPlainText());
        file.close();
    } else if (auto sr = dynamic_cast<const StringShaderResource*>(obj_->getResource().get())) {
        // get the non-const version from the manager.
        auto res = ShaderManager::getPtr()->getShaderResource(sr->key());
        if (auto editable = dynamic_cast<StringShaderResource*>(res.get())) {
            editable->setSource(utilqt::fromQString(shadercode_->toPlainText()));
        }
    }
    shadercode_->document()->setModified(false);
}

std::string ShaderWidget::getFileName(const ShaderObject* obj) {
    if (auto fr = dynamic_cast<const FileShaderResource*>(obj->getResource().get())) {
        return fr->file();
    } else {
        return obj->getFileName();
    }
}

}  // namespace inviwo
