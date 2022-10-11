/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/util/colorconversion.h>      // for hsv2rgb, rgb2hsv
#include <inviwo/core/util/glmvec.h>               // for vec4, vec3
#include <inviwo/core/util/logcentral.h>           // for LogCentral, LogWarn
#include <inviwo/core/util/raiiutils.h>            // for KeepTrueWhileInScope
#include <modules/opengl/shader/shadermanager.h>   // for ShaderManager
#include <modules/opengl/shader/shaderobject.h>    // for ShaderObject
#include <modules/opengl/shader/shaderresource.h>  // for StringShaderResource
#include <modules/openglqt/glslsyntaxhighlight.h>  // for setGLSLSyntaxHighlight, GLSLSyntaxHigh...
#include <modules/qtwidgets/codeedit.h>            // for CodeEdit
#include <modules/qtwidgets/inviwodockwidget.h>    // for InviwoDockWidget
#include <modules/qtwidgets/inviwoqtutils.h>       // for toQString, fromQString, emToPx

#include <algorithm>    // for max, any_of, count
#include <cstddef>      // for size_t
#include <limits>       // for numeric_limits
#include <string>       // for string, to_string, basic_string, basic...
#include <string_view>  // for string_view, basic_string_view, hash
#include <tuple>        // for tuple_element<>::type
#include <type_traits>  // for add_const<>::type, remove_extent_t
#include <utility>      // for pair

#include <QAction>         // for QAction
#include <QDockWidget>     // for QDockWidget
#include <QEvent>          // for QEvent, QEvent::FocusIn
#include <QFlags>          // for QFlags
#include <QIcon>           // for QIcon, QIcon::Normal, QIcon::Off, QIco...
#include <QKeySequence>    // for QKeySequence, QKeySequence::Redo, QKey...
#include <QList>           // for QList
#include <QObject>         // for QObject
#include <QPixmap>         // for QPixmap
#include <QPlainTextEdit>  // for QPlainTextEdit
#include <QSizeF>          // for QSizeF
#include <QString>         // for operator+, QString
#include <QTextDocument>   // for QTextDocument
#include <QWidget>         // for QWidget
#include <Qt>              // for operator|, WidgetWithChildrenShortcut
#include <fmt/core.h>      // for format
#include <fmt/format.h>    // for compile_string_to_view, FMT_STRING
#include <glm/vec3.hpp>    // for vec, vec<>::(anonymous)
#include <glm/vec4.hpp>    // for vec<>::(anonymous)

#include <QMainWindow>     // for QMainWindow
#include <QMessageBox>     // for QMessageBox, operator|, QMessageBox::Save
#include <QScrollBar>      // for QScrollBar
#include <QSignalBlocker>  // for QSignalBlocker
#include <QToolBar>        // for QToolBar

class QCloseEvent;

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

    apply_ = toolBar->addAction(QIcon(":/svgicons/run-script.svg"), tr("&Apply Changes"));
    apply_->setToolTip(
        "Replace the ShaderResource in the shader with a with this content. The "
        "changes will only affect this shader and will not be persistent.");
    apply_->setShortcut(Qt::CTRL | Qt::Key_R);
    apply_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    mainWindow->addAction(apply_);
    connect(apply_, &QAction::triggered, this, &ShaderWidget::apply);

    save_ = toolBar->addAction(QIcon(":/svgicons/save.svg"), tr("&Save Shader"));
    save_->setToolTip(
        "If a FileShaderResource saves changes to disk, changes will be persistent "
        "and all shaders using the file will be reloaded. If a StringShaderResource "
        "updates the string, changes will affect all shaders using the resource but will not be "
        "persistent.");
    save_->setShortcut(QKeySequence::Save);
    save_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    mainWindow->addAction(save_);
    connect(save_, &QAction::triggered, this, &ShaderWidget::save);

    revert_ = toolBar->addAction(QIcon(":/svgicons/revert.svg"), tr("Revert"));
    revert_->setToolTip("Revert changes");
    revert_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    revert_->setEnabled(false);
    QObject::connect(shadercode_, &QPlainTextEdit::modificationChanged, revert_,
                     &QAction::setEnabled);
    QObject::connect(revert_, &QAction::triggered, this, &ShaderWidget::revert);

    QPixmap enabled(":/svgicons/precompiled-enabled.svg");
    QPixmap disabled(":/svgicons/precompiled-disabled.svg");
    QIcon preicon;
    preicon.addPixmap(enabled, QIcon::Normal, QIcon::Off);
    preicon.addPixmap(disabled, QIcon::Normal, QIcon::On);

    preprocess_ = toolBar->addAction(preicon, "Show Preprocessed Shader");
    preprocess_->setChecked(false);
    preprocess_->setCheckable(true);

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

bool ShaderWidget::eventFilter(QObject* obj, QEvent* event) {
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
    } else {
        LogWarn(fmt::format(
            "Could not save. The ShaderResource \"{}\" was not found in the ShaderManager. It "
            "needs to be registered with the ShaderManager for saving to work.",
            obj_->getResource()->key()));
    }
}

void ShaderWidget::apply() {
    ignoreNextUpdate_ = true;

    if (!orignal_) {
        orignal_ = obj_->getResource();
    }

    auto tmp = std::make_shared<StringShaderResource>(
        "[tmp]", utilqt::fromQString(shadercode_->toPlainText()));
    obj_->setResource(tmp);
    setWindowTitle("<tmp file>[*]");
    revert_->setEnabled(true);
    shadercode_->document()->setModified(false);
}

void ShaderWidget::revert() {
    if (orignal_) {
        ignoreNextUpdate_ = true;
        obj_->setResource(orignal_);
        setWindowTitle(utilqt::toQString(obj_->getFileName()) + "[*]");
        orignal_ = nullptr;
    }
    updateState();
}

void ShaderWidget::updateState() {
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
            const auto&& [tag, num] = obj_->resolveLine(line - 1);
            const auto pos = tag.find_last_of('/');
            const auto file = std::string_view{tag}.substr(pos + 1);

            return fmt::format(FMT_STRING("{0:<{2}}{1:>{3}}"), file, num, width + 1u, numberSize);
        });
        shadercode_->setAnnotationSpace(
            [width, numberSize](int) { return static_cast<int>(width + 1 + numberSize); });

        shadercode_->setLineAnnotationColor([this](int line, vec4 org) {
            const auto&& [tag, num] = obj_->resolveLine(line - 1);
            if (auto pos = tag.find_first_of('['); pos != std::string::npos) {
                const auto resource = std::string_view{tag}.substr(0, pos);

                auto hsv = color::rgb2hsv(vec3(org));
                auto randH = static_cast<double>(std::hash<std::string_view>{}(resource)) /
                             static_cast<double>(std::numeric_limits<size_t>::max());

                auto adjusted =
                    vec4{color::hsv2rgb(vec3(static_cast<float>(randH), std::max(0.75f, hsv.y),
                                             std::max(0.25f, hsv.z))),
                         org.w};
                return adjusted;
            } else {
                return org;
            }
        });

    } else {
        shadercode_->setLineAnnotation([](int line) { return std::to_string(line); });
        shadercode_->setLineAnnotationColor([](int, vec4 org) { return org; });
        shadercode_->setAnnotationSpace([](int maxDigits) { return maxDigits; });
    }

    shadercode_->setReadOnly(checked);
    save_->setEnabled(!checked);
    apply_->setEnabled(!checked);
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
        std::string msg = "The shader source has been modified, do you want to reload its content?";

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
