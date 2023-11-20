/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <modules/qtwidgets/properties/filepropertywidgetqt.h>

#include <inviwo/core/properties/fileproperty.h>              // for FileProperty
#include <inviwo/core/properties/propertysemantics.h>         // for operator==, PropertySemantics
#include <inviwo/core/util/filedialogstate.h>                 // for FileMode, AcceptMode, Accep...
#include <inviwo/core/util/fileextension.h>                   // for FileExtension
#include <inviwo/core/util/filesystem.h>                      // for directoryExists, getFileDir...
#include <modules/qtwidgets/editablelabelqt.h>                // for EditableLabelQt
#include <modules/qtwidgets/filepathlineeditqt.h>             // for FilePathLineEditQt
#include <modules/qtwidgets/inviwofiledialog.h>               // for InviwoFileDialog
#include <modules/qtwidgets/inviwoqtutils.h>                  // for fromQString, toQString
#include <modules/qtwidgets/properties/propertywidgetqt.h>    // for PropertyWidgetQt
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>  // for TextEditorDockWidget

#include <string>  // for basic_string, operator+
#include <vector>  // for vector

#include <QDesktopServices>  // for QDesktopServices
#include <QDragEnterEvent>   // for QDragEnterEvent
#include <QDragMoveEvent>    // for QDragMoveEvent
#include <QDropEvent>        // for QDropEvent
#include <QHBoxLayout>       // for QHBoxLayout
#include <QIcon>             // for QIcon
#include <QList>             // for QList
#include <QMimeData>         // for QMimeData
#include <QSizePolicy>       // for QSizePolicy
#include <QString>           // for QString
#include <QToolButton>       // for QToolButton
#include <QUrl>              // for QUrl, QUrl::TolerantMode
#include <QWidget>           // for QWidget

namespace inviwo {

class PropertyEditorWidget;

FilePropertyWidgetQt::FilePropertyWidgetQt(FileProperty* property)
    : PropertyWidgetQt(property), property_(property), lineEdit_{new FilePathLineEditQt(this)} {

    setFocusPolicy(lineEdit_->focusPolicy());
    setFocusProxy(lineEdit_);
    setAcceptDrops(true);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);
    hLayout->addWidget(new EditableLabelQt(this, property_));
    hWidgetLayout_ = new QHBoxLayout();

    {
        hWidgetLayout_->setContentsMargins(0, 0, 0, 0);
        auto widget = new QWidget();
        widget->setLayout(hWidgetLayout_);
        auto sp = widget->sizePolicy();
        sp.setHorizontalStretch(3);
        widget->setSizePolicy(sp);
        hLayout->addWidget(widget);
    }

    {
        connect(lineEdit_, &FilePathLineEditQt::pathChanged, this,
                [this](const std::filesystem::path& path) { property_->set(path); });
        auto sp = lineEdit_->sizePolicy();
        sp.setHorizontalStretch(3);
        lineEdit_->setSizePolicy(sp);
        hWidgetLayout_->addWidget(lineEdit_);
    }

    {
        auto revealButton = new QToolButton(this);
        revealButton->setIcon(QIcon(":/svgicons/about-enabled.svg"));
        hWidgetLayout_->addWidget(revealButton);
        connect(revealButton, &QToolButton::pressed, this, [&]() {
            const auto dir = std::filesystem::is_directory(property_->get())
                                 ? property_->get()
                                 : property_->get().parent_path();

            QDesktopServices::openUrl(
                QUrl(utilqt::toQString("file:///" + dir.string()), QUrl::TolerantMode));
        });
    }

    {
        auto openButton = new QToolButton(this);
        openButton->setIcon(QIcon(":/svgicons/open.svg"));
        hWidgetLayout_->addWidget(openButton);
        connect(openButton, &QToolButton::pressed, this, &FilePropertyWidgetQt::setPropertyValue);
    }

    if (property_->getSemantics() == PropertySemantics::TextEditor) {
        addEditor();
    }

    updateFromProperty();
}

void FilePropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<TextEditorDockWidget>(property_);
}

void FilePropertyWidgetQt::addEditor() {
    auto edit = new QToolButton();
    edit->setIcon(QIcon(":/svgicons/edit.svg"));
    edit->setToolTip("Edit String");
    hWidgetLayout_->addWidget(edit);
    connect(edit, &QToolButton::clicked, this, [this]() {
        if (!editor_) initEditor();
        editor_->updateFromProperty();
        editor_->setVisible(true);
    });
}

void FilePropertyWidgetQt::setPropertyValue() {
    InviwoFileDialog fileDialog(this, property_->getDisplayName(), property_->getContentType(),
                                property_->get());

    fileDialog.setAcceptMode(property_->getAcceptMode());
    fileDialog.setFileMode(property_->getFileMode());
    fileDialog.addExtensions(property_->getNameFilters());
    fileDialog.setSelectedExtension(property_->getSelectedExtension());

    if (fileDialog.exec()) {
        property_->set(fileDialog.getSelectedFile(), fileDialog.getSelectedFileExtension());
    }
}

void FilePropertyWidgetQt::dropEvent(QDropEvent* drop) {
    auto mimeData = drop->mimeData();
    if (!mimeData->urls().empty()) {
        auto url = mimeData->urls().front();
        property_->set(utilqt::toPath(url.toLocalFile()));
        drop->accept();
    } else {
        drop->ignore();
    }
}

void FilePropertyWidgetQt::dragEnterEvent(QDragEnterEvent* event) {
    const auto mimeData = event->mimeData();
    if (!mimeData->urls().empty()) {
        auto url = mimeData->urls().front();
        const auto file = utilqt::toPath(url.toLocalFile());

        switch (property_->getFileMode()) {
            case FileMode::AnyFile:
            case FileMode::ExistingFile:
            case FileMode::ExistingFiles: {
                if (property_->matchesAnyNameFilter(file)) {
                    event->accept();
                    return;
                }
                break;
            }
            case FileMode::Directory: {
                if (std::filesystem::is_directory(file)) {
                    event->accept();
                    return;
                }
                break;
            }
        }
    }

    event->ignore();
}

void FilePropertyWidgetQt::dragMoveEvent(QDragMoveEvent* event) {
    event->setAccepted(event->mimeData()->hasUrls());
}

bool FilePropertyWidgetQt::requestFile() {
    setPropertyValue();
    return !property_->get().empty();
}

PropertyEditorWidget* FilePropertyWidgetQt::getEditorWidget() const { return editor_.get(); }

bool FilePropertyWidgetQt::hasEditorWidget() const { return editor_ != nullptr; }

void FilePropertyWidgetQt::updateFromProperty() {
    lineEdit_->setPath(property_->get());
    lineEdit_->setAcceptMode(property_->getAcceptMode());
    lineEdit_->setFileMode(property_->getFileMode());
}

}  // namespace inviwo
