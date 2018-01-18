/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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
    : PropertyEditorWidgetQt(property, "Edit " + property->getDisplayName(), "TextEditorDockWidget")
    , fileProperty_{dynamic_cast<FileProperty*>(property)}
    , stringProperty_{dynamic_cast<StringProperty*>(property)} {

    QMainWindow* mainWindow = new QMainWindow();
    mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow);

    editor_ = new QTextEdit(nullptr);
    editor_->setObjectName("SourceCodeEditor");
    editor_->setReadOnly(false);
    editor_->setWordWrapMode(QTextOption::NoWrap);
    editor_->createStandardContextMenu();
    mainWindow->setCentralWidget(editor_);

    // setting a monospace font explicitely is necessary despite providing a font-family in css
    // Otherwise, the editor will not feature a fixed-width font face.
    QFont fixedFont("Monospace");
    fixedFont.setPointSize(10);
    fixedFont.setStyleHint(QFont::TypeWriter);
    editor_->setFont(fixedFont);

    QString bgString;
    if (property->getSemantics() == PropertySemantics::ShaderEditor) {
        syntaxHighligther_ = SyntaxHighligther::createSyntaxHighligther<GLSL>(editor_->document());
    } else {
        syntaxHighligther_ = SyntaxHighligther::createSyntaxHighligther<None>(editor_->document());
    }

    // set background of text editor matching syntax highlighting
    const QColor bgColor = syntaxHighligther_->getBackgroundColor();
    QString styleSheet(QString("QTextEdit#%1 { background-color: %2; }")
                           .arg(editor_->objectName())
                           .arg(bgColor.name()));
    editor_->setStyleSheet(styleSheet);

    {
        auto save = toolBar->addAction(QIcon(":/icons/save.png"), tr("&Save"));
        save->setShortcut(QKeySequence::Save);
        save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(save);
        connect(save, &QAction::triggered, this, [this]() {
            if (editor_->document()->isModified()) {
                if (fileProperty_) {
                    if (auto f = filesystem::ofstream(fileProperty_->get())) {
                        f << utilqt::fromQString(editor_->toPlainText());
                    }
                } else if (stringProperty_) {
                    stringProperty_->set(utilqt::fromQString(editor_->toPlainText()));
                }
            }
        });
    }

    {
        auto undo = toolBar->addAction(QIcon(":/icons/undo.png"), tr("Undo"));
        undo->setShortcut(QKeySequence::Undo);
        undo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(undo);
        connect(undo, &QAction::triggered, editor_, &QTextEdit::undo);
    }

    {
        auto redo = toolBar->addAction(QIcon(":/icons/redo.png"), tr("Redo"));
        redo->setShortcut(QKeySequence::Redo);
        redo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(redo);
        connect(redo, &QAction::triggered, editor_, &QTextEdit::redo);
    }

    {

        auto reload = toolBar->addAction(QIcon(":/icons/button_cancel-bw.png"), tr("Revert"));
        reload->setToolTip("Revert changes");
        reload->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(reload);
        connect(reload, &QAction::triggered, [this]() { updateFromProperty(); });
    }

    resize(QSize(500, 500));  // default size
    setVisible(false);
    updateFromProperty();
    loadState();
}

SyntaxHighligther* TextEditorDockWidget::getSyntaxHighligther() { return syntaxHighligther_; }

void TextEditorDockWidget::updateFromProperty() {
    if (fileProperty_) {
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
}

void TextEditorDockWidget::closeEvent(QCloseEvent* e) {
    if (stringProperty_->get() != utilqt::fromQString(editor_->toPlainText())) {
        auto ret = QMessageBox::warning(
            this, utilqt::toQString("Edit " + stringProperty_->getDisplayName()),
            tr("The document has been modified.\n"
               "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard);

        if (ret == QMessageBox::Save) {
            if (fileProperty_) {
                if (auto f = filesystem::ofstream(fileProperty_->get())) {
                    f << utilqt::fromQString(editor_->toPlainText());
                }
            } else if (stringProperty_) {
                stringProperty_->set(utilqt::fromQString(editor_->toPlainText()));
            }
        }
    }
    PropertyEditorWidgetQt::closeEvent(e);
}

void TextEditorDockWidget::onSetDisplayName(Property*, const std::string& displayName) {
    setWindowTitle(QString::fromStdString(displayName));
}

void TextEditorDockWidget::setReadOnly(bool readonly) { editor_->setReadOnly(readonly); }

}  // namespace inviwo
