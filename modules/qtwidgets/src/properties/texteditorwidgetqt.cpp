/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <inviwo/core/properties/fileproperty.h>                  // for FileProperty
#include <inviwo/core/properties/property.h>                      // for Property
#include <inviwo/core/properties/stringproperty.h>                // for StringProperty
#include <inviwo/core/util/filedialogstate.h>                     // for AcceptMode, AcceptMode:...
#include <inviwo/core/util/filesystem.h>                          // for ifstream, ofstream
#include <modules/qtwidgets/codeedit.h>                           // for CodeEdit
#include <modules/qtwidgets/editorfileobserver.h>                 // for EditorFileObserver
#include <modules/qtwidgets/inviwofiledialog.h>                   // for InviwoFileDialog
#include <modules/qtwidgets/inviwoqtutils.h>                      // for fromQString, toQString
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>  // for PropertyEditorWidgetQt

#include <fstream>  // for basic_ifstream, basic_o...
#include <vector>   // for vector

#include <QAction>         // for QAction
#include <QFileDialog>     // for QFileDialog, QFileDialo...
#include <QIcon>           // for QIcon
#include <QKeySequence>    // for QKeySequence, QKeySeque...
#include <QList>           // for QList
#include <QMainWindow>     // for QMainWindow
#include <QMessageBox>     // for QMessageBox, operator|
#include <QObject>         // for QObject
#include <QPlainTextEdit>  // for QPlainTextEdit
#include <QSizeF>          // for QSizeF
#include <QString>         // for operator+, QString
#include <QStringList>     // for QStringList
#include <QTextDocument>   // for QTextDocument
#include <QToolBar>        // for QToolBar
#include <Qt>              // for WidgetWithChildrenShortcut

class QCloseEvent;

namespace inviwo {

class FileExtension;

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

    editor_ = new CodeEdit{this};
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

        auto saveas = toolBar->addAction(QIcon(":/svgicons/save-as.svg"), tr("&Save Script As..."));
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

SyntaxHighlighter& TextEditorDockWidget::getSyntaxHighlighter() {
    return editor_->syntaxHighlighter();
}

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
