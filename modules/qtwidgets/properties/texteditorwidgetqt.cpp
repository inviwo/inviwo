/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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
#include <modules/qtwidgets/qtwidgetssettings.h>
#include <modules/qtwidgets/inviwofiledialog.h>

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
    , observer_{*this, util::getInviwoApplication(property)} {

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

    QObject::connect(editor_, &QTextEdit::textChanged, this, [this]() { setTitle(true); });

    QString bgString;
    if (property->getSemantics() == PropertySemantics::ShaderEditor) {
        syntaxHighligther_ = SyntaxHighligther::createSyntaxHighligther<GLSL>(editor_->document());
        if (auto app = util::getInviwoApplication(property)) {
            app->getSettingsByType<QtWidgetsSettings>()->glslSyntax_.onChange(
                this, &TextEditorDockWidget::updateStyle);
        }
    } else if (property->getSemantics() == PropertySemantics::PythonEditor) {
        syntaxHighligther_ =
            SyntaxHighligther::createSyntaxHighligther<Python>(editor_->document());
        if (auto app = util::getInviwoApplication(property)) {
            app->getSettingsByType<QtWidgetsSettings>()->pythonSyntax_.onChange(
                this, &TextEditorDockWidget::updateStyle);
        }
    } else {
        syntaxHighligther_ = SyntaxHighligther::createSyntaxHighligther<None>(editor_->document());
    }

    updateStyle();

    {
        auto save = toolBar->addAction(QIcon(":/icons/save.png"), tr("&Save"));
        save->setShortcut(QKeySequence::Save);
        save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(save);
        connect(save, &QAction::triggered, this, [this]() {
            if (editor_->document()->isModified()) {
                editor_->document()->setModified(false);
                if (fileProperty_) {
                    observer_.stopAllObservation();
                    if (auto f = filesystem::ofstream(fileProperty_->get())) {
                        f << utilqt::fromQString(editor_->toPlainText());
                    }
                    observer_.startFileObservation(fileProperty_->get());
                } else if (stringProperty_) {
                    stringProperty_->set(utilqt::fromQString(editor_->toPlainText()));
                }
                setTitle(false);
            }
        });
    }

    if (fileProperty_) {
        fileCallback_ = fileProperty_->onChange([this]() {
            observer_.stopAllObservation();
            fileChanged();
            observer_.startFileObservation(fileProperty_->get());
        });
        observer_.startFileObservation(fileProperty_->get());

        auto saveas = toolBar->addAction(QIcon(":/icons/saveas.png"), tr("&Save Script As..."));
        saveas->setShortcut(QKeySequence::SaveAs);
        saveas->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        saveas->setToolTip("Save Script As...");
        mainWindow->addAction(saveas);
        connect(saveas, &QAction::triggered, this, [this]() {
            InviwoFileDialog saveFileDialog(this, "Save File ...", fileProperty_->getContentType());

            saveFileDialog.setFileMode(FileMode::AnyFile);
            saveFileDialog.setAcceptMode(AcceptMode::Save);
            saveFileDialog.setConfirmOverwrite(true);

            for (const auto& filter : fileProperty_->getNameFilters()) {
                saveFileDialog.addExtension(filter);
            }
            if (saveFileDialog.exec()) {
                observer_.stopFileObservation(fileProperty_->get());
                QString path = saveFileDialog.selectedFiles().at(0);

                if (auto f = filesystem::ofstream(utilqt::fromQString(path))) {
                    f << utilqt::fromQString(editor_->toPlainText());
                }
                editor_->document()->setModified(false);
                fileProperty_->set(utilqt::fromQString(path));
            }
        });
    } else if (stringProperty_) {
        stringCallback_ = stringProperty_->onChange([this]() { fileChanged(); });
    }

    {
        auto revert = toolBar->addAction(QIcon(":/icons/revert.png"), tr("Revert"));
        revert->setToolTip("Revert changes");
        revert->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        mainWindow->addAction(revert);
        connect(revert, &QAction::triggered, [this]() { updateFromProperty(); });
    }

    {
        auto undo = toolBar->addAction(QIcon(":/icons/undo.png"), tr("Undo"));
        undo->setShortcut(QKeySequence::Undo);
        undo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        undo->setEnabled(false);
        mainWindow->addAction(undo);
        connect(undo, &QAction::triggered, editor_, &QTextEdit::undo);
        connect(editor_, &QTextEdit::undoAvailable, undo, &QAction::setEnabled);
    }

    {
        auto redo = toolBar->addAction(QIcon(":/icons/redo.png"), tr("Redo"));
        redo->setShortcut(QKeySequence::Redo);
        redo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        redo->setEnabled(false);
        mainWindow->addAction(redo);
        connect(redo, &QAction::triggered, editor_, &QTextEdit::redo);
        connect(editor_, &QTextEdit::redoAvailable, redo, &QAction::setEnabled);
    }

    resize(QSize(500, 500));  // default size
    setVisible(false);
    updateFromProperty();
    loadState();
}

SyntaxHighligther* TextEditorDockWidget::getSyntaxHighligther() { return syntaxHighligther_; }

TextEditorDockWidget::~TextEditorDockWidget() {
}

void TextEditorDockWidget::updateStyle() {
    auto getBGColorAndSizeAndFont = [&]() {
        if (auto app = util::getInviwoApplication(property_)) {
            auto settings = app->getSettingsByType<QtWidgetsSettings>();
            if (property_->getSemantics() == PropertySemantics::ShaderEditor) {
                auto color = settings->glslBackgroundColor_.get();
                auto size = settings->glslFontSize_.get();
                auto family = settings->glslFont_.get();
                return std::make_tuple(color, size, family);
            } else if (property_->getSemantics() == PropertySemantics::PythonEditor) {
                auto color = settings->pyBGColor_.get();
                auto size = settings->pyFontSize_.get();
                auto family = settings->pythonFont_.get();
                return std::make_tuple(color, size, family);
            }
        }
        return std::make_tuple(ivec4(0xb0, 0xb0, 0xbc, 255), 11, std::string{"Courier New"});
    };

    auto csf = getBGColorAndSizeAndFont();

    std::stringstream ss;
    ss << "background-color: rgb(" << std::get<0>(csf).r << ", " << std::get<0>(csf).g
       << ", " << std::get<0>(csf).b << ");\n"
       << "font-size: " << std::get<1>(csf) << "px;\n"
       << "font-family: " << std::get<2>(csf) << ";\n";
    editor_->setStyleSheet(ss.str().c_str());
    syntaxHighligther_->rehighlight();

    auto font = editor_->font();
    QFontMetrics metrics(font);
    editor_->setTabStopWidth(4 * metrics.width(' '));
}

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
    setTitle(false);
}

void TextEditorDockWidget::closeEvent(QCloseEvent* e) {
    if (fileProperty_ && fileCallback_) {
        observer_.stopAllObservation();
        fileProperty_->removeOnChange(fileCallback_);
    }
    if (stringProperty_ && stringCallback_) {
        stringProperty_->removeOnChange(stringCallback_);
    }

    if (editor_->document()->isModified()) {
        auto ret = QMessageBox::warning(this, "Save",
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

void TextEditorDockWidget::onSetDisplayName(Property*, const std::string&) {
    setTitle(editor_->document()->isModified());
}

void TextEditorDockWidget::setReadOnly(bool readonly) { editor_->setReadOnly(readonly); }

void TextEditorDockWidget::setTitle(bool modified) {
    if (fileProperty_) {
        setWindowTitle(utilqt::toQString("Edit " + fileProperty_->get() + (modified ? "*" : "")));
    } else if (stringProperty_) {
        setWindowTitle(
            utilqt::toQString("Edit " + stringProperty_->getDisplayName() + (modified ? "*" : "")));
    }
}

void TextEditorDockWidget::fileChanged() {
    if (editor_->document()->isModified()) {
        auto ret =
            QMessageBox::warning(this, "Update",
                                 tr("The file has been modified.\n"
                                    "Do you want to update the editor, discarding your changes?"),
                                 QMessageBox::Yes | QMessageBox::Cancel);

        if (ret == QMessageBox::Yes) {
            updateFromProperty();
        }
    } else {
        updateFromProperty();
    }
}

TextEditorDockWidget::ScriptObserver::ScriptObserver(TextEditorDockWidget& widget,
                                                     InviwoApplication* app)
    : FileObserver(app), widget_(widget) {}

void TextEditorDockWidget::ScriptObserver::fileChanged(const std::string&) {
    widget_.fileChanged();
};

}  // namespace inviwo
