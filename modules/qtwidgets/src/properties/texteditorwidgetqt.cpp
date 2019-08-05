/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/qtwidgets/properties/texteditorwidgetqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/properties/property.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <modules/qtwidgets/properties/syntaxhighlighter.h>
#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/buttonpropertywidgetqt.h>
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>

#include <modules/qtwidgets/codeedit.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTextDocument>
#include <QTextBlock>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QToolButton>
#include <QToolBar>
#include <QTextEdit>
#include <QDesktopServices>
#include <QTextStream>
#include <QMessageBox>
#include <QFont>
#include <warn/pop>

namespace inviwo {

TextEditorDockWidget::TextEditorDockWidget(Property* property)
    : PropertyEditorWidgetQt(property, "Edit", "TextEditorDockWidget")
    , fileProperty_{dynamic_cast<FileProperty*>(property)}
    , stringProperty_{dynamic_cast<StringProperty*>(property)}
    , fileObserver_{this, "Text Editor"} {

    QMainWindow* mainWindow = new QMainWindow();
    mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow);

    if (property->getSemantics() == PropertySemantics::ShaderEditor) {
        editor_ = new CodeEdit{GLSL};
    } else if (property->getSemantics() == PropertySemantics::PythonEditor) {
        editor_ = new CodeEdit{Python};
    } else {
        editor_ = new CodeEdit{None};
    }
    mainWindow->setCentralWidget(editor_);

    QObject::connect(editor_, &CodeEdit::modificationChanged, this,
                     [this](bool b) { setWindowModified(b); });
    editor_->installEventFilter(&fileObserver_);

    fileObserver_.setReloadFileCallback([this]() { updateFromProperty(); });
    fileObserver_.setModifiedCallback([this](bool m) { editor_->document()->setModified(m); });

    {
        auto save = toolBar->addAction(QIcon(":/svgicons/save.svg"), tr("&Save"));
        save->setShortcut(QKeySequence::Save);
        save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(save);
        connect(save, &QAction::triggered, this, &TextEditorDockWidget::save);
    }

    if (fileProperty_) {
        propertyCallback_ = fileProperty_->onChangeScoped([this]() { propertyModified(); });

        auto saveas = toolBar->addAction(QIcon(":/svgicons/saveas.svg"), tr("&Save Script As..."));
        saveas->setShortcut(QKeySequence::SaveAs);
        saveas->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        saveas->setToolTip("Save Script As...");
        mainWindow->addAction(saveas);
        connect(saveas, &QAction::triggered, this, [this]() {
            InviwoFileDialog saveFileDialog(this, "Save File ...", fileProperty_->getContentType());

            saveFileDialog.setFileMode(FileMode::AnyFile);
            saveFileDialog.setAcceptMode(AcceptMode::Save);
            saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

            for (const auto& filter : fileProperty_->getNameFilters()) {
                saveFileDialog.addExtension(filter);
            }
            if (saveFileDialog.exec()) {
                auto path = utilqt::fromQString(saveFileDialog.selectedFiles().at(0));

                saveToFile(path);
                fileObserver_.ignoreNextUpdate();
                fileObserver_.setFileName(path);
                editor_->document()->setModified(false);
                fileProperty_->set(path);
            }
        });
    } else if (stringProperty_) {
        propertyCallback_ = stringProperty_->onChangeScoped([this]() { propertyModified(); });
    }

    {
        auto revert = toolBar->addAction(QIcon(":/svgicons/revert.svg"), tr("Revert"));
        revert->setToolTip("Revert changes");
        revert->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        revert->setEnabled(false);
        mainWindow->addAction(revert);
        QObject::connect(revert, &QAction::triggered, [this]() { updateFromProperty(); });
        QObject::connect(editor_, &QPlainTextEdit::modificationChanged, revert,
                         &QAction::setEnabled);
    }

    {
        auto undo = toolBar->addAction(QIcon(":/svgicons/undo.svg"), tr("Undo"));
        undo->setShortcut(QKeySequence::Undo);
        undo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        undo->setEnabled(false);
        mainWindow->addAction(undo);
        connect(undo, &QAction::triggered, editor_, &CodeEdit::undo);
        connect(editor_, &CodeEdit::undoAvailable, undo, &QAction::setEnabled);
    }

    {
        auto redo = toolBar->addAction(QIcon(":/svgicons/redo.svg"), tr("Redo"));
        redo->setShortcut(QKeySequence::Redo);
        redo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        redo->setEnabled(false);
        mainWindow->addAction(redo);
        connect(redo, &QAction::triggered, editor_, &CodeEdit::redo);
        connect(editor_, &CodeEdit::redoAvailable, redo, &QAction::setEnabled);
    }

    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size
    setVisible(false);
    updateFromProperty();
    loadState();
}

SyntaxHighligther* TextEditorDockWidget::getSyntaxHighligther() { return syntaxHighligther_; }

TextEditorDockWidget::~TextEditorDockWidget() = default;

void TextEditorDockWidget::updateFromProperty() {
    if (fileProperty_) {
        fileObserver_.setFileName(fileProperty_->get());
        if (auto f = filesystem::ifstream(fileProperty_->get())) {
            std::stringstream ss;
            ss << f.rdbuf();
            editor_->setPlainText(utilqt::toQString(ss.str()));
        } else {
            editor_->setPlainText("");
        }
    } else if (stringProperty_) {
        editor_->setPlainText(utilqt::toQString(stringProperty_->get()));
    }
    editor_->document()->setModified(false);
    updateWindowTitle();
}

void TextEditorDockWidget::closeEvent(QCloseEvent* e) {
    propertyCallback_.reset();
    if (editor_->document()->isModified()) {
        QMessageBox msgBox(QMessageBox::Question, "Text Editor",
                           "Do you want to save unsaved changes?",
                           QMessageBox::Save | QMessageBox::Discard, this);
        int retval = msgBox.exec();
        if (retval == QMessageBox::Save) {
            save();
        } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
            return;
        }
    }
    PropertyEditorWidgetQt::closeEvent(e);
}

void TextEditorDockWidget::onSetDisplayName(Property*, const std::string&) { updateWindowTitle(); }

void TextEditorDockWidget::setReadOnly(bool readonly) { editor_->setReadOnly(readonly); }

void TextEditorDockWidget::updateWindowTitle() {
    auto str = [&]() -> std::string {
        if (fileProperty_) {
            return "Text Editor - " + fileProperty_->get();
        } else if (stringProperty_) {
            return "Text Editor - " + stringProperty_->getDisplayName();
        }
        return "Text Editor";
    }();

    setWindowTitle(utilqt::toQString(str) + "[*]");
}

void TextEditorDockWidget::propertyModified() {
    if (editor_->document()->isModified() && fileProperty_) {
        QMessageBox msgBox(QMessageBox::Question, "Text Editor - FileProperty Changed",
                           "The FileProperty has been updated. Do you want to save any changes "
                           "before the editor is updated?",
                           QMessageBox::Save | QMessageBox::Discard, this);
        msgBox.setWindowModality(Qt::WindowModal);
        if (msgBox.exec() == QMessageBox::Save) {
            save();
        }
    }
    updateFromProperty();
}

void TextEditorDockWidget::save() {
    if (!editor_->document()->isModified()) {
        return;
    }

    if (fileProperty_) {
        fileObserver_.ignoreNextUpdate();
        saveToFile(fileProperty_->get());
    } else if (stringProperty_) {
        stringProperty_->set(utilqt::fromQString(editor_->toPlainText()));
    }
    editor_->document()->setModified(false);
}

void TextEditorDockWidget::saveToFile(const std::string& filename) {
    if (auto f = filesystem::ofstream(filename)) {
        f << utilqt::fromQString(editor_->toPlainText());
    }
}

}  // namespace inviwo
