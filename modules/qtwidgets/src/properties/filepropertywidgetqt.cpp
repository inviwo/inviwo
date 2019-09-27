/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/properties/propertyowner.h>
#include <modules/qtwidgets/filepathlineeditqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QSettings>
#include <QUrl>
#include <QDropEvent>
#include <QMimeData>
#include <QHBoxLayout>
#include <QToolButton>
#include <warn/pop>

namespace inviwo {

FilePropertyWidgetQt::FilePropertyWidgetQt(FileProperty* property)
    : PropertyWidgetQt(property), property_(property), lineEdit_{new FilePathLineEditQt(this)} {

    setFocusPolicy(lineEdit_->focusPolicy());
    setFocusProxy(lineEdit_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);
    setAcceptDrops(true);

    hLayout->addWidget(new EditableLabelQt(this, property_));
    auto hWidgetLayout = new QHBoxLayout();

    {
        hWidgetLayout->setContentsMargins(0, 0, 0, 0);
        auto widget = new QWidget();
        widget->setLayout(hWidgetLayout);
        auto sp = widget->sizePolicy();
        sp.setHorizontalStretch(3);
        widget->setSizePolicy(sp);
        hLayout->addWidget(widget);
    }

    {
        connect(lineEdit_, &FilePathLineEditQt::editingFinished, this, [this]() {
            // editing is done, sync property with contents
            if (lineEdit_->isModified()) {
                property_->set(lineEdit_->getPath());
                if (!property_->getSelectedExtension().matches(property_->get())) {
                    property_->setSelectedExtension(FileExtension::all());
                }
            }
        });
        auto sp = lineEdit_->sizePolicy();
        sp.setHorizontalStretch(3);
        lineEdit_->setSizePolicy(sp);
        hWidgetLayout->addWidget(lineEdit_);
    }

    {
        auto revealButton = new QToolButton(this);
        revealButton->setIcon(QIcon(":/svgicons/about-enabled.svg"));
        hWidgetLayout->addWidget(revealButton);
        connect(revealButton, &QToolButton::pressed, this, [&]() {
            auto dir = filesystem::directoryExists(property_->get())
                           ? property_->get()
                           : filesystem::getFileDirectory(property_->get());

            QDesktopServices::openUrl(
                QUrl(utilqt::toQString("file:///" + dir), QUrl::TolerantMode));
        });
    }

    {
        auto openButton = new QToolButton(this);
        openButton->setIcon(QIcon(":/svgicons/open.svg"));
        hWidgetLayout->addWidget(openButton);
        connect(openButton, &QToolButton::pressed, this, &FilePropertyWidgetQt::setPropertyValue);
    }

    if (property_->getSemantics() == PropertySemantics::TextEditor ||
        property_->getSemantics() == PropertySemantics::ShaderEditor ||
        property_->getSemantics() == PropertySemantics::PythonEditor) {
        auto edit = new QToolButton();
        edit->setIcon(QIcon(":/svgicons/edit.svg"));
        edit->setToolTip("Edit String");
        hWidgetLayout->addWidget(edit);
        connect(edit, &QToolButton::clicked, this, [this]() {
            if (!editor_) {
                editor_ = std::make_unique<TextEditorDockWidget>(property_);
            }
            editor_->updateFromProperty();
            editor_->setVisible(true);
        });
    }

    updateFromProperty();
}

void FilePropertyWidgetQt::setPropertyValue() {
    const std::string filename{property_->get()};

    InviwoFileDialog fileDialog(this, property_->getDisplayName(), property_->getContentType(),
                                filename);

    fileDialog.setAcceptMode(property_->getAcceptMode());
    fileDialog.setFileMode(property_->getFileMode());
    fileDialog.addExtensions(property_->getNameFilters());
    fileDialog.setSelectedExtension(property_->getSelectedExtension());

    if (fileDialog.exec()) {
        property_->set(fileDialog.getSelectedFile(), fileDialog.getSelectedFileExtension());
    }

    updateFromProperty();
}

void FilePropertyWidgetQt::dropEvent(QDropEvent* drop) {
    auto mimeData = drop->mimeData();
    if (mimeData->hasUrls()) {
        if (mimeData->urls().size() > 0) {
            auto url = mimeData->urls().first();
            property_->set(utilqt::fromQString(url.toLocalFile()));

            drop->accept();
        }
    }
}

void FilePropertyWidgetQt::dragEnterEvent(QDragEnterEvent* event) {
    switch (property_->getAcceptMode()) {
        case AcceptMode::Save: {
            event->ignore();
            return;
        }
        case AcceptMode::Open: {
            if (event->mimeData()->hasUrls()) {
                auto mimeData = event->mimeData();
                if (mimeData->hasUrls()) {
                    if (mimeData->urls().size() > 0) {
                        auto url = mimeData->urls().first();
                        auto file = url.toLocalFile().toStdString();

                        switch (property_->getFileMode()) {
                            case FileMode::AnyFile:
                            case FileMode::ExistingFile:
                            case FileMode::ExistingFiles: {
                                auto ext = toLower(filesystem::getFileExtension(file));
                                for (const auto& filter : property_->getNameFilters()) {
                                    if (filter.extension_ == ext) {
                                        event->accept();
                                        return;
                                    }
                                }
                                break;
                            }

                            case FileMode::Directory:
                            case FileMode::DirectoryOnly: {
                                if (filesystem::directoryExists(file)) {
                                    event->accept();
                                    return;
                                }
                                break;
                            }
                        }
                    }
                }
            }
            event->ignore();
            return;
        }
    }
}

void FilePropertyWidgetQt::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

bool FilePropertyWidgetQt::requestFile() {
    setPropertyValue();
    return !property_->get().empty();
}

PropertyEditorWidget* FilePropertyWidgetQt::getEditorWidget() const { return editor_.get(); }

bool FilePropertyWidgetQt::hasEditorWidget() const { return editor_ != nullptr; }

void FilePropertyWidgetQt::updateFromProperty() {
    lineEdit_->setPath(property_->get());
    lineEdit_->setModified(false);
}

}  // namespace inviwo
