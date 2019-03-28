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

#include <modules/qtwidgets/properties/multifilepropertywidgetqt.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/filedialogstate.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/filepathlineeditqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>

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

MultiFilePropertyWidgetQt::MultiFilePropertyWidgetQt(MultiFileProperty* property)
    : PropertyWidgetQt(property)
    , property_(property)
    , lineEdit_{new FilePathLineEditQt(this)}
    , label_{new EditableLabelQt(this, property_)} {

    setFocusPolicy(lineEdit_->focusPolicy());
    setFocusProxy(lineEdit_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);
    setAcceptDrops(true);

    hLayout->addWidget(label_);

    QHBoxLayout* hWidgetLayout = new QHBoxLayout();
    hWidgetLayout->setContentsMargins(0, 0, 0, 0);
    QWidget* widget = new QWidget();
    widget->setLayout(hWidgetLayout);

    connect(lineEdit_, &FilePathLineEditQt::editingFinished, this, [this]() {
        // editing is done, sync property with contents
        if (lineEdit_->isModified()) {
            property_->set(lineEdit_->getPath());
        }
    });
#if defined(IVW_DEBUG)
    QObject::connect(lineEdit_, &LineEditQt::editingCanceled, this, [this]() {
        // undo textual changes by resetting the contents of the line edit
        ivwAssert(
            lineEdit_->getPath() == (!property_->get().empty() ? property_->get().front() : ""),
            "MultiFilePropertyWidgetQt: paths not equal after canceling edit");
    });
#endif  // IVW_DEBUG

    QSizePolicy sp = lineEdit_->sizePolicy();
    sp.setHorizontalStretch(3);
    lineEdit_->setSizePolicy(sp);
    hWidgetLayout->addWidget(lineEdit_);

    auto revealButton = new QToolButton(this);
    revealButton->setIcon(QIcon(":/svgicons/about-enabled.svg"));
    hWidgetLayout->addWidget(revealButton);
    connect(revealButton, &QToolButton::pressed, this, [&]() {
        auto fileName = (!property_->get().empty() ? property_->get().front() : "");
        auto dir = filesystem::directoryExists(fileName) ? fileName
                                                         : filesystem::getFileDirectory(fileName);

        QDesktopServices::openUrl(
            QUrl(QString::fromStdString("file:///" + dir), QUrl::TolerantMode));
    });

    auto openButton = new QToolButton(this);
    openButton->setIcon(QIcon(":/svgicons/open.svg"));
    hWidgetLayout->addWidget(openButton);
    connect(openButton, &QToolButton::pressed, this, &MultiFilePropertyWidgetQt::setPropertyValue);

    sp = widget->sizePolicy();
    sp.setHorizontalStretch(3);
    widget->setSizePolicy(sp);
    hLayout->addWidget(widget);

    updateFromProperty();
}

void MultiFilePropertyWidgetQt::setPropertyValue() {
    std::string fileName = (!property_->get().empty() ? property_->get().front() : "");

    // Setup Extensions
    std::vector<FileExtension> filters = property_->getNameFilters();
    InviwoFileDialog importFileDialog(this, property_->getDisplayName(),
                                      property_->getContentType(), fileName);

    for (const auto& filter : filters) importFileDialog.addExtension(filter);

    importFileDialog.setAcceptMode(property_->getAcceptMode());
    importFileDialog.setFileMode(property_->getFileMode());

    auto ext = property_->getSelectedExtension();
    if (!ext.empty()) importFileDialog.setSelectedExtension(ext);

    if (importFileDialog.exec()) {
        std::vector<std::string> filenames;
        for (auto item : importFileDialog.selectedFiles()) {
            filenames.push_back(item.toStdString());
        }
        property_->set(filenames);
        property_->setSelectedExtension(importFileDialog.getSelectedFileExtension());
    }

    updateFromProperty();
}

void MultiFilePropertyWidgetQt::dropEvent(QDropEvent* drop) {
    auto mineData = drop->mimeData();
    if (mineData->hasUrls()) {
        if (mineData->urls().size() > 0) {
            auto url = mineData->urls().first();
            property_->set(url.toLocalFile().toStdString());

            drop->accept();
        }
    }
}

void MultiFilePropertyWidgetQt::dragEnterEvent(QDragEnterEvent* event) {
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

void MultiFilePropertyWidgetQt::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool MultiFilePropertyWidgetQt::requestFile() {
    setPropertyValue();
    return !property_->get().empty();
}

void MultiFilePropertyWidgetQt::updateFromProperty() {
    if (!property_->get().empty()) {
        lineEdit_->setPath(property_->get().front());
    } else {
        lineEdit_->setPath("");
    }
    lineEdit_->setModified(false);
}

}  // namespace inviwo
