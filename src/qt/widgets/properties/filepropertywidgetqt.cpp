/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/widgets/properties/filepropertywidgetqt.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QSettings>
#include <QUrl>
#include <inviwo/qt/widgets/inviwofiledialog.h>

namespace inviwo {

FilePropertyWidgetQt::FilePropertyWidgetQt(FileProperty* property)
    : PropertyWidgetQt(property), property_(property) {
    generateWidget();
    updateFromProperty();
}

void FilePropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);

    label_ = new EditableLabelQt(this, property_->getDisplayName());
    hLayout->addWidget(label_);

    QHBoxLayout* hWidgetLayout = new QHBoxLayout();
    hWidgetLayout->setContentsMargins(0,0,0,0);
    QWidget* widget = new QWidget();
    widget->setLayout(hWidgetLayout);

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setReadOnly(true);
    openButton_ = new QToolButton(this);
    openButton_->setIcon(QIcon(":/icons/open.png"));
    hWidgetLayout->addWidget(lineEdit_);
    hWidgetLayout->addWidget(openButton_);
    hLayout->addWidget(widget);

    setLayout(hLayout);

    connect(label_, SIGNAL(textChanged()), this, SLOT(setPropertyDisplayName()));
    connect(openButton_, SIGNAL(pressed()), this, SLOT(setPropertyValue()));
}

void FilePropertyWidgetQt::setPropertyValue() {
    
    QString dataDir_ = QString::fromStdString(
        InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_DATA));
    
    // Setup default path
    QString path;

    if (property_->get() != "")
        path = QDir(QString::fromStdString(property_->get())).absolutePath();
    else
        path = QDir(dataDir_).absolutePath();

    // Setup Extensions
    std::vector<std::string> filters = property_->getNameFilters();
    InviwoFileDialog importFileDialog(this, property_->getDisplayName(),
                                      property_->getContentType());
    

    for (std::vector<std::string>::const_iterator it = filters.begin(); it != filters.end(); ++it)
        importFileDialog.addExtension(*it);


    switch (property_->getAcceptMode()) {
        case FileProperty::AcceptSave:
            importFileDialog.setAcceptMode(QFileDialog::AcceptSave);
            break;

        case FileProperty::AcceptOpen:
            importFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
            break;

        default:
            importFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    }

    switch (property_->getFileMode()) {
        case FileProperty::AnyFile:
            importFileDialog.setFileMode(QFileDialog::AnyFile);
            break;

        case FileProperty::ExistingFile:
            importFileDialog.setFileMode(QFileDialog::ExistingFile);
            break;

        case FileProperty::Directory:
            importFileDialog.setFileMode(QFileDialog::Directory);
            break;

        case FileProperty::ExistingFiles:
            importFileDialog.setFileMode(QFileDialog::ExistingFiles);
            break;

        case FileProperty::DirectoryOnly:
            importFileDialog.setFileMode(QFileDialog::DirectoryOnly);
            break;

        default:
            importFileDialog.setFileMode(QFileDialog::AnyFile);
    }

    if (importFileDialog.exec()) {
        QString path = importFileDialog.selectedFiles().at(0);
        property_->set(path.toLocal8Bit().constData());
    }

    updateFromProperty();
}

void FilePropertyWidgetQt::updateFromProperty() {
    lineEdit_->setText(QFileInfo(QString::fromStdString(property_->get())).fileName());
    lineEdit_->setDisabled(property_->getReadOnly());
}

void FilePropertyWidgetQt::setPropertyDisplayName() {
    property_->setDisplayName(label_->getText());
}

}  // namespace
