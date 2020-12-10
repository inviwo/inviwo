/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2020 Inviwo Foundation
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

#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>

#include <inviwo/core/common/inviwoapplication.h>

#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shadermanager.h>

#include <modules/qtwidgets/syntaxhighlighter.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/codeedit.h>

#include <modules/openglqt/glslsyntaxhighlight.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QScrollBar>
#include <warn/pop>

namespace inviwo {

ShaderWidget::ShaderWidget(ShaderObject* obj, QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(obj->getFileName()) + "[*]", parent, "ShaderEditorWidget")
    , obj_{obj}
    , shaderObjOnChange_{obj->onChange([this](ShaderObject*) { shaderObjectChanged(); })} {

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

    shadercode_ = new CodeEdit{this};
    auto settings = InviwoApplication::getPtr()->getSettingsByType<GLSLSyntaxHighlight>();
    codeCallbacks_ = utilqt::setGLSLSyntaxHighlight(shadercode_->syntaxHighlighter(), *settings);

    shadercode_->setObjectName("shaderwidgetcode");
    shadercode_->setPlainText(utilqt::toQString(obj->print(false, false)));

    save_ = toolBar->addAction(QIcon(":/svgicons/save.svg"), tr("&Save Shader"));
    save_->setShortcut(QKeySequence::Save);
    save_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    mainWindow->addAction(save_);
    connect(save_, &QAction::triggered, this, &ShaderWidget::save);

    QPixmap enabled(":/svgicons/precompiled-enabled.svg");
    QPixmap disabled(":/svgicons/precompiled-disabled.svg");
    QIcon preicon;
    preicon.addPixmap(enabled, QIcon::Normal, QIcon::Off);
    preicon.addPixmap(disabled, QIcon::Normal, QIcon::On);

    preprocess_ = toolBar->addAction(preicon, "Show Preprocessed Shader");
    preprocess_->setChecked(false);
    preprocess_->setCheckable(true);

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

    QObject::connect(preprocess_, &QAction::toggled, this, [=](bool checked) {
        if (checked && shadercode_->document()->isModified()) {
            QMessageBox msgBox(
                QMessageBox::Question, "Shader Editor", "Do you want to save unsaved changes?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

            int retval = msgBox.exec();
            if (retval == static_cast<int>(QMessageBox::Save)) {
                this->save();
            } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
                QSignalBlocker block(preprocess_);
                preprocess_->setChecked(false);
                return;
            }
        }
        updateState();
    });
    QObject::connect(revert, &QAction::triggered, this, &ShaderWidget::updateState);
    QObject::connect(shadercode_, &QPlainTextEdit::modificationChanged, this,
                     &QDockWidget::setWindowModified);

    shadercode_->installEventFilter(this);
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

inline bool ShaderWidget::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        if (fileChangedInBackground_) {
            queryReloadFile();
        }
        return false;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void ShaderWidget::save() {
    ignoreNextUpdate_ = true;

    // get the non-const version from the manager.
    if (auto resource = ShaderManager::getPtr()->getShaderResource(obj_->getResource()->key())) {
        resource->setSource(utilqt::fromQString(shadercode_->toPlainText()));
        shadercode_->document()->setModified(false);
    }
}

inline void ShaderWidget::updateState() {
    const bool checked = preprocess_->isChecked();
    const auto code = obj_->print(false, checked);

    const auto vPosition = shadercode_->verticalScrollBar()->value();
    const auto hPosition = shadercode_->horizontalScrollBar()->value();
    shadercode_->setPlainText(utilqt::toQString(code));
    shadercode_->verticalScrollBar()->setValue(vPosition);
    shadercode_->horizontalScrollBar()->setValue(hPosition);

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
            out << std::left << std::setw(width + 1u) << file << std::right << std::setw(numberSize)
                << info.second;
            return out.str();
        });
        shadercode_->setAnnotationSpace(
            [width, numberSize](int) { return static_cast<int>(width + 1 + numberSize); });
    } else {
        shadercode_->setLineAnnotation([](int line) { return std::to_string(line); });
        shadercode_->setAnnotationSpace([](int maxDigits) { return maxDigits; });
    }

    shadercode_->setReadOnly(checked);
    save_->setEnabled(!checked);
    preprocess_->setText(checked ? "Show Plain Shader Only" : "Show Preprocessed Shader");
    shadercode_->document()->setModified(false);
}

inline void ShaderWidget::queryReloadFile() {
    if (preprocess_->isChecked()) {
        util::KeepTrueWhileInScope guard{&reloadQueryInProgress_};
        updateState();
        fileChangedInBackground_ = false;
        return;
    }

    auto children = findChildren<QWidget*>();
    auto focus =
        std::any_of(children.begin(), children.end(), [](auto w) { return w->hasFocus(); });
    if (focus && fileChangedInBackground_ && !reloadQueryInProgress_) {
        util::KeepTrueWhileInScope guard{&reloadQueryInProgress_};
        std::string msg =
            "The shader source has been modified, do you want to reload its contents?";

        QMessageBox msgBox(QMessageBox::Question, "Shader Editor", utilqt::toQString(msg),
                           QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setWindowModality(Qt::WindowModal);

        if (msgBox.exec() == QMessageBox::Yes) {
            updateState();
        } else {
            shadercode_->document()->setModified(true);
        }
        fileChangedInBackground_ = false;
    }
}

void ShaderWidget::shaderObjectChanged() {
    if (ignoreNextUpdate_) {
        ignoreNextUpdate_ = false;
        return;
    }

    fileChangedInBackground_ = true;
    queryReloadFile();
}

}  // namespace inviwo
