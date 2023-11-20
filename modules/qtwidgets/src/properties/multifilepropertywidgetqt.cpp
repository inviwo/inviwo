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

#include <modules/qtwidgets/properties/multifilepropertywidgetqt.h>

#include <inviwo/core/properties/multifileproperty.h>       // for MultiFileProperty
#include <inviwo/core/util/filedialogstate.h>               // for FileMode, AcceptMode, AcceptM...
#include <inviwo/core/util/fileextension.h>                 // for FileExtension
#include <inviwo/core/util/filesystem.h>                    // for directoryExists, getFileDirec...
#include <inviwo/core/util/assertion.h>                     // IWYU pragma: keep
#include <modules/qtwidgets/editablelabelqt.h>              // for EditableLabelQt
#include <modules/qtwidgets/filepathlineeditqt.h>           // for FilePathLineEditQt
#include <modules/qtwidgets/inviwofiledialog.h>             // for InviwoFileDialog
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt
#include <modules/qtwidgets/inviwoqtutils.h>

#include <string>  // for string, basic_string, operator+
#include <vector>  // for vector, __vector_base<>::valu...

#include <QDesktopServices>  // for QDesktopServices
#include <QDragEnterEvent>   // for QDragEnterEvent
#include <QDragMoveEvent>    // for QDragMoveEvent
#include <QDropEvent>        // for QDropEvent
#include <QHBoxLayout>       // for QHBoxLayout
#include <QIcon>             // for QIcon
#include <QList>             // for QList, QList<>::iterator
#include <QMimeData>         // for QMimeData
#include <QSizePolicy>       // for QSizePolicy
#include <QString>           // for QString
#include <QStringList>       // for QStringList
#include <QToolButton>       // for QToolButton
#include <QUrl>              // for QUrl, QUrl::TolerantMode
#include <QWidget>           // for QWidget

class QHBoxLayout;

namespace inviwo {

MultiFilePropertyWidgetQt::MultiFilePropertyWidgetQt(MultiFileProperty* property)
    : PropertyWidgetQt(property), property_(property), lineEdit_{new FilePathLineEditQt(this)} {

    setFocusPolicy(lineEdit_->focusPolicy());
    setFocusProxy(lineEdit_);
    setAcceptDrops(true);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);
    hLayout->addWidget(new EditableLabelQt(this, property_));
    QHBoxLayout* hWidgetLayout = new QHBoxLayout();

    {
        hWidgetLayout->setContentsMargins(0, 0, 0, 0);
        QWidget* widget = new QWidget();
        widget->setLayout(hWidgetLayout);
        auto sp = widget->sizePolicy();
        sp.setHorizontalStretch(3);
        widget->setSizePolicy(sp);
        hLayout->addWidget(widget);
    }

    {
        connect(lineEdit_, &FilePathLineEditQt::pathChanged, this,
                [this](const std::filesystem::path& path) { property_->set(path); });
        QSizePolicy sp = lineEdit_->sizePolicy();
        sp.setHorizontalStretch(3);
        lineEdit_->setSizePolicy(sp);
        hWidgetLayout->addWidget(lineEdit_);
    }

    {
        auto revealButton = new QToolButton(this);
        revealButton->setIcon(QIcon(":/svgicons/about-enabled.svg"));
        hWidgetLayout->addWidget(revealButton);
        connect(revealButton, &QToolButton::pressed, this, [&]() {
            const auto fileName = (!property_->get().empty() ? property_->get().front() : "");
            const auto dir =
                std::filesystem::is_directory(fileName) ? fileName : fileName.parent_path();

            QDesktopServices::openUrl(
                QUrl(QString::fromStdString("file:///" + dir.string()), QUrl::TolerantMode));
        });
    }
    {
        auto openButton = new QToolButton(this);
        openButton->setIcon(QIcon(":/svgicons/open.svg"));
        hWidgetLayout->addWidget(openButton);
        connect(openButton, &QToolButton::pressed, this,
                &MultiFilePropertyWidgetQt::setPropertyValue);
    }

    updateFromProperty();
}

void MultiFilePropertyWidgetQt::setPropertyValue() {
    const auto fileName = property_->front() ? *property_->front() : std::filesystem::path{};

    InviwoFileDialog fileDialog(this, property_->getDisplayName(), property_->getContentType(),
                                fileName);

    fileDialog.setAcceptMode(property_->getAcceptMode());
    fileDialog.setFileMode(property_->getFileMode());
    fileDialog.addExtensions(property_->getNameFilters());
    fileDialog.setSelectedExtension(property_->getSelectedExtension());

    if (fileDialog.exec()) {
        property_->set(fileDialog.getSelectedFiles(), fileDialog.getSelectedFileExtension());
    }
}

void MultiFilePropertyWidgetQt::dropEvent(QDropEvent* drop) {
    auto mimeData = drop->mimeData();
    if (!mimeData->urls().empty()) {
        std::vector<std::filesystem::path> paths;
        for (auto&& url : mimeData->urls()) {
            paths.push_back(utilqt::toPath(url.toLocalFile()));
        }
        property_->set(paths);
        drop->accept();
    } else {
        drop->ignore();
    }
}

void MultiFilePropertyWidgetQt::dragEnterEvent(QDragEnterEvent* event) {
    auto mimeData = event->mimeData();
    bool matches = !mimeData->urls().empty();
    for (auto&& url : mimeData->urls()) {
        const auto file = utilqt::toPath(url.toLocalFile());

        switch (property_->getFileMode()) {
            case FileMode::AnyFile:
            case FileMode::ExistingFile:
            case FileMode::ExistingFiles: {
                if (!property_->matchesAnyNameFilter(file)) {
                    matches = false;
                }
                break;
            }
            case FileMode::Directory: {
                if (!std::filesystem::is_directory(file)) {
                    matches = false;
                }
                break;
            }
        }
    }

    event->setAccepted(matches);
}

void MultiFilePropertyWidgetQt::dragMoveEvent(QDragMoveEvent* event) {
    event->setAccepted(event->mimeData()->hasUrls());
}

bool MultiFilePropertyWidgetQt::requestFile() {
    setPropertyValue();
    return !property_->get().empty();
}

void MultiFilePropertyWidgetQt::updateFromProperty() {
    lineEdit_->setPath(property_->front() ? *property_->front() : std::filesystem::path{});
    lineEdit_->setAcceptMode(property_->getAcceptMode());
    lineEdit_->setFileMode(property_->getFileMode());
}

}  // namespace inviwo
