/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/qtwidgets/networkannotationwidget.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkannotations.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/qt/editor/networkeditor.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/lineeditqt.h>
#include <modules/qtwidgets/codeedit.h>
#include <modules/qtwidgets/inviwodockwidget.h>

#include <memory>
#include <functional>
#include <utility>

#include <QScrollArea>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QColorDialog>
#include <QComboBox>
#include <QMainWindow>
#include <QToolBar>
#include <QMessageBox>

namespace inviwo {

class EditorDockWidget : public InviwoDockWidget {
public:
    explicit EditorDockWidget(QString title, QWidget* parent = nullptr)
        : InviwoDockWidget{std::move(title), parent, "AnnotationEditorDockWidget"}
        , editor_{new CodeEdit{this}} {

        auto* mainWindow = new QMainWindow();
        mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
        auto* toolBar = new QToolBar();
        toolBar->setObjectName("TextEditorWidgetToolBar");
        mainWindow->addToolBar(toolBar);
        toolBar->setFloatable(false);
        toolBar->setMovable(false);
        mainWindow->setCentralWidget(editor_);
        setWidget(mainWindow);
        resize(utilqt::emToPx(this, QSizeF(60, 60)));

        {
            auto* save = toolBar->addAction(QIcon(":/svgicons/save.svg"), tr("&Save"));
            save->setShortcut(QKeySequence::Save);
            save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            mainWindow->addAction(save);
            connect(save, &QAction::triggered, this, [this]() {
                if (!editor_->document()->isModified()) {
                    return;
                }
                if (saveFunc_) {
                    util::exceptionGuard([this]() { saveFunc_(editor_->toPlainText()); });
                }
                editor_->document()->setModified(false);
            });
        }

        {
            auto* revert = toolBar->addAction(QIcon(":/svgicons/revert.svg"), tr("Revert"));
            revert->setToolTip("Revert changes");
            revert->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            revert->setEnabled(false);
            mainWindow->addAction(revert);
            QObject::connect(revert, &QAction::triggered, [this]() {
                if (revertFunc_) {
                    util::exceptionGuard([this]() { revertFunc_(); });
                }
            });
            QObject::connect(editor_, &QPlainTextEdit::modificationChanged, revert,
                             &QAction::setEnabled);
        }

        {
            auto* undo = toolBar->addAction(QIcon(":/svgicons/undo.svg"), tr("Undo"));
            undo->setShortcut(QKeySequence::Undo);
            undo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            undo->setEnabled(false);
            mainWindow->addAction(undo);
            connect(undo, &QAction::triggered, editor_, &CodeEdit::undo);
            connect(editor_, &CodeEdit::undoAvailable, undo, &QAction::setEnabled);
        }

        {
            auto* redo = toolBar->addAction(QIcon(":/svgicons/redo.svg"), tr("Redo"));
            redo->setShortcut(QKeySequence::Redo);
            redo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            redo->setEnabled(false);
            mainWindow->addAction(redo);
            connect(redo, &QAction::triggered, editor_, &CodeEdit::redo);
            connect(editor_, &CodeEdit::redoAvailable, redo, &QAction::setEnabled);
        }
    }

    void setSaveCallback(std::function<void(const QString&)> saveFunc) {
        saveFunc_ = std::move(saveFunc);
    }

    void setRevertCallback(std::function<void()> revertFunc) {
        revertFunc_ = std::move(revertFunc);
    }

    void setText(const QString& text) {
        const auto oldPos = editor_->textCursor().position();

        editor_->setPlainText(text);
        editor_->document()->setModified(false);

        auto cursor = editor_->textCursor();
        cursor.movePosition(QTextCursor::MoveOperation::End);
        const auto max = cursor.position();
        cursor.setPosition(std::min(oldPos, max));
        editor_->setTextCursor(cursor);
        editor_->ensureCursorVisible();
    }

    QString getText() const { return editor_->toPlainText(); }

    void save() {
        if (!editor_->document()->isModified()) {
            return;
        }
        if (saveFunc_) {
            util::exceptionGuard([this]() { saveFunc_(editor_->toPlainText()); });
        }
        editor_->document()->setModified(false);
    }

protected:
    virtual void closeEvent(QCloseEvent* e) override {
        if (editor_->document()->isModified()) {
            QMessageBox msgBox(QMessageBox::Question, "Text Editor",
                               "Do you want to save unsaved changes?",
                               QMessageBox::Save | QMessageBox::Discard, this);
            const int retval = msgBox.exec();
            if (retval == QMessageBox::Save) {
                save();
            } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
                return;
            }
        }
        InviwoDockWidget::closeEvent(e);
    }

private:
    CodeEdit* editor_;
    std::function<void(const QString&)> saveFunc_;
    std::function<void()> revertFunc_;
};

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
NetworkAnnotationWidget::NetworkAnnotationWidget(ProcessorNetwork* network, QWidget* parent)
    : InviwoDockWidget{tr("Network Annotations"), parent, "NetworkAnnotationsWidget"}
    , mainWidget_{new QWidget{}}
    , scrollArea_{new QScrollArea{this}}
    , network_{network}
    , lineEditTitle_{new LineEditQt{this}}
    , lineEditDesc_{new LineEditQt{this}}
    , comboAlignment_{new QComboBox{this}}
    , numberTextWidth_{new NumberWidget<int>{}}
    , btnColor_{new QToolButton{}}
    , colorDialog_{nullptr}
    , editorDockWidget_{
          new EditorDockWidget{"Annotation Description", utilqt::getApplicationMainWindow()}} {

    annotationClearHandle_ =
        network_->getApplication()->getWorkspaceManager()->onClear([this]() { clear(); });

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(45, 80)));  // default size

    {
        QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        sp.setVerticalStretch(1);
        sp.setHorizontalStretch(1);
        setSizePolicy(sp);
    }

    const auto space = utilqt::refSpacePx(this);
    const auto propertySpacing = utilqt::emToPx(mainWidget_, PropertyWidgetQt::spacingEm);

    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(utilqt::emToPx(this, 30));
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
    // Scrollbars are overlayed in different way on mac...
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setContentsMargins(0, space, 0, space);

    lineEditTitle_->setPlaceholderText("<unset>");
    connect(lineEditTitle_, &LineEditQt::editingFinished, this, [this]() {
        if (annotationIndex_.has_value()) {
            if (!lineEditTitle_->text().isEmpty()) {
                annotation_.title = utilqt::fromQString(lineEditTitle_->text());
            } else {
                annotation_.title.reset();
            }
            emit modifiedAnnotation(*annotationIndex_, annotation_);
        }
    });
    connect(lineEditTitle_, &LineEditQt::editingCanceled, [this]() {
        // undo textual changes by resetting the contents of the line edit
        const QSignalBlocker blocker(lineEditTitle_);
        if (annotationIndex_.has_value()) {
            lineEditTitle_->setText(utilqt::toQString(annotation_.title.value_or("")));
        } else {
            lineEditTitle_->clear();
        }

        lineEditTitle_->clearFocus();
    });

    editorDockWidget_->setFloating(true);
    editorDockWidget_->setSaveCallback([this](const QString& text) {
        if (setDescription(text)) {
            lineEditDesc_->setText(utilqt::toQString(
                annotation_.description.transform([](auto& desc) { return desc.markdown; })
                    .value_or("")));
        }
    });
    editorDockWidget_->setRevertCallback([this]() {
        if (annotationIndex_.has_value()) {
            editorDockWidget_->setText(utilqt::toQString(
                annotation_.description.transform([](auto& desc) { return desc.markdown; })
                    .value_or("")));
        }
    });
    lineEditDesc_->setPlaceholderText("<empty>");
    {
        QSizePolicy sp = lineEditDesc_->sizePolicy();
        sp.setHorizontalStretch(3);
        lineEditDesc_->setSizePolicy(sp);
    }
    connect(lineEditDesc_, &LineEditQt::editingFinished, this, [this]() {
        if (setDescription(lineEditDesc_->text())) {
            editorDockWidget_->setText(utilqt::toQString(
                annotation_.description.transform([](auto& desc) { return desc.markdown; })
                    .value_or("")));
        }
    });
    connect(lineEditDesc_, &LineEditQt::editingCanceled, [this]() {
        // undo textual changes by resetting the contents of the line edit
        const QSignalBlocker blocker(lineEditDesc_);
        if (annotationIndex_.has_value()) {
            lineEditDesc_->setText(utilqt::toQString(
                annotation_.description.transform([](auto& desc) { return desc.markdown; })
                    .value_or("")));
        } else {
            lineEditDesc_->clear();
        }

        lineEditDesc_->clearFocus();
    });

    auto* editorBtn = new QToolButton();
    editorBtn->setIcon(QIcon(":/svgicons/edit.svg"));
    editorBtn->setToolTip("Edit text");
    connect(editorBtn, &QToolButton::clicked, this,
            [this]() { editorDockWidget_->setVisible(true); });

    auto* descriptionLayout = new QHBoxLayout();
    descriptionLayout->setContentsMargins(0, 0, 0, 0);
    descriptionLayout->setSpacing(propertySpacing);
    descriptionLayout->addWidget(lineEditDesc_);
    descriptionLayout->addWidget(editorBtn);

    QSizePolicy spBtn = sizePolicy();
    spBtn.setHorizontalPolicy(QSizePolicy::Fixed);
    spBtn.setVerticalPolicy(QSizePolicy::Fixed);
    spBtn.setHorizontalStretch(0);

    btnColor_->setSizePolicy(spBtn);
    btnColor_->setAutoRaise(true);
    btnColor_->setObjectName("ColorButton");
    btnColor_->setFocusPolicy(Qt::ClickFocus);
    connect(btnColor_, &QToolButton::clicked, this, &NetworkAnnotationWidget::openColorDialog);

    auto* colorLayout = new QHBoxLayout();
    colorLayout->setContentsMargins(0, 0, 0, 0);
    colorLayout->setSpacing(propertySpacing);
    colorLayout->addWidget(btnColor_);
    colorLayout->addStretch();

    comboAlignment_->addItems(QStringList{{"Left", "Top", "Right", "Bottom"}});
    connect(comboAlignment_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (annotationIndex_.has_value() && annotation_.description.has_value()) {
            const auto alignment = static_cast<Description::Alignment>(index);
            if (alignment != annotation_.description->alignment) {
                annotation_.description->alignment = alignment;
                emit modifiedAnnotation(*annotationIndex_, annotation_);
            }
        }
    });

    numberTextWidth_->setMinValue(0, ConstraintBehavior::Immutable);
    numberTextWidth_->setMaxValue(250, ConstraintBehavior::Ignore);
    numberTextWidth_->initValue(100);
    connect(numberTextWidth_, &NumberWidget<int>::valueChanged, this, [this]() {
        if (annotationIndex_.has_value() && annotation_.description.has_value() &&
            numberTextWidth_->getValue() != annotation_.description->width) {
            annotation_.description->width = numberTextWidth_->getValue();
            emit modifiedAnnotation(*annotationIndex_, annotation_);
        }
    });

    auto* layout = new QGridLayout{mainWidget_};
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 3);
    layout->setVerticalSpacing(propertySpacing);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(propertySpacing * 2, propertySpacing, propertySpacing,
                               propertySpacing);

    layout->addWidget(new QLabel{"Title"}, 0, 0);
    layout->addWidget(lineEditTitle_, 0, 1);

    layout->addWidget(new QLabel{"Color"}, 1, 0);
    layout->addLayout(colorLayout, 1, 1);

    layout->addWidget(new QLabel{"Description"}, 2, 0);
    layout->addLayout(descriptionLayout, 2, 1);

    layout->addWidget(new QLabel{"Alignment"}, 3, 0);
    layout->addWidget(comboAlignment_, 3, 1);

    layout->addWidget(new QLabel{"Text Width (px)"}, 4, 0);
    layout->addWidget(numberTextWidth_, 4, 1);

    scrollArea_->setWidget(mainWidget_);
    setWidget(scrollArea_);

    mainWidget_->setVisible(annotationIndex_.has_value());
    editorDockWidget_->setVisible(false);
}

NetworkAnnotationWidget::~NetworkAnnotationWidget() = default;

void NetworkAnnotationWidget::showAnnotation(size_t index, const NetworkAnnotation& annotation) {
    annotationIndex_ = index;
    annotation_ = annotation;

    updateWidgets();
    if (annotationIndex_.has_value()) {
        QWidget::raise();
    }
}

void NetworkAnnotationWidget::hideAnnotation(size_t index) {
    if (index == annotationIndex_) {
        annotationIndex_.reset();
        annotation_ = {};
        editorDockWidget_->setVisible(false);

        updateWidgets();
    }
}

void NetworkAnnotationWidget::clear() {
    annotationIndex_.reset();
    mainWidget_->setVisible(false);
}

void NetworkAnnotationWidget::updateWidgets() {
    mainWidget_->setVisible(annotationIndex_.has_value());
    if (!annotationIndex_.has_value()) return;

    lineEditTitle_->setText(utilqt::toQString(annotation_.title.value_or("")));

    const QString description{utilqt::toQString(
        annotation_.description.transform([](auto& desc) { return desc.markdown; }).value_or(""))};
    editorDockWidget_->setText(description);
    lineEditDesc_->setText(description);

    const auto alignment =
        annotation_.description.transform([](auto& desc) { return desc.alignment; })
            .value_or(Description::Alignment::Left);
    comboAlignment_->setCurrentIndex(static_cast<int>(alignment));

    const int descriptionWidth =
        annotation_.description.transform([](auto& desc) { return desc.width; }).value_or(100);
    numberTextWidth_->setValue(descriptionWidth);

    updateColorButton(utilqt::toQColor(annotation_.color));
}

void NetworkAnnotationWidget::updateColorButton(QColor color) {
    const QColor topColor = color.lighter();
    const QColor bottomColor = color.darker();

    btnColor_->setStyleSheet(QString("QToolButton {"
                                     "    border: 1px solid transparent;"
                                     "    background-color: %1;"
                                     "    border-radius: 3px;"
                                     "    width: 2ex;"
                                     "    height: 2ex;"
                                     "}"
                                     "QToolButton:hover {"
                                     "    border: 1px solid #268bd2;"
                                     "    background-color: qlineargradient(x1:0, \
                                     y1:0, x2:0, y2:1, stop:0 %2, stop:0.1 %1, stop:0.9 %1, \
                                     stop:1 %3);"
                                     "}"
                                     "QToolButton:pressed {"
                                     "    border: 1px solid #268bd2;"
                                     "    background-color: qlineargradient(x1:0, y1:0, x2:0, \
                                     y2:1, stop:1 %2, stop:0.9 %1, stop:0.1 %1, stop:0 %3);"
                                     "}")
                                 .arg(color.name())
                                 .arg(topColor.name())
                                 .arg(bottomColor.name()));
}

void NetworkAnnotationWidget::openColorDialog() {
    if (!colorDialog_) {
        colorDialog_ = new QColorDialog(this);
#ifdef __APPLE__
        // hide the dialog, due to some Mac issues (OSX Bug workaround)
        colorDialog_->hide();
#endif  // __APPLE__

        colorDialog_->setAttribute(Qt::WA_DeleteOnClose, false);
        colorDialog_->setOption(QColorDialog::ShowAlphaChannel, false);
        colorDialog_->setOption(QColorDialog::NoButtons, true);
        colorDialog_->setWindowModality(Qt::NonModal);
        QObject::connect(colorDialog_, &QColorDialog::currentColorChanged, this,
                         [this](QColor color) {
                             if (annotationIndex_.has_value()) {
                                 annotation_.color = utilqt::tovec3(color);
                                 updateColorButton(color);
                                 emit modifiedAnnotation(*annotationIndex_, annotation_);
                             }
                         });

        colorDialog_->installEventFilter(new utilqt::WidgetCloseEventFilter(this));
    }

#ifdef __APPLE__
    colorDialog_->hide();  // OSX Bug workaround
#endif                     // __APPLE__
    {
        const QSignalBlocker block{colorDialog_};
        colorDialog_->setWindowTitle("Annotation Color");
        colorDialog_->setCurrentColor(utilqt::toQColor(annotation_.color));
    }
    colorDialog_->show();
}

bool NetworkAnnotationWidget::setDescription(const QString& text) {
    if (annotationIndex_.has_value()) {
        auto str = utilqt::fromQString(text);
        auto description = annotation_.description.value_or(Description{});
        if (description.markdown != str) {
            if (!str.empty()) {
                description.markdown = std::move(str);
                annotation_.description = std::move(description);
            } else {
                annotation_.description.reset();
            }
            emit modifiedAnnotation(*annotationIndex_, annotation_);
            return true;
        }
    }
    return false;
}

}  // namespace inviwo
