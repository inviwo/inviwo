/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/qt/widgets/properties/directorypropertywidgetqt.h>

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

DirectoryPropertyWidgetQt::DirectoryPropertyWidgetQt(DirectoryProperty* property)
    : PropertyWidgetQt(property), property_(property) {
    generateWidget();
    updateFromProperty();
}

void DirectoryPropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();

    directoryLabel_ = new EditableLabelQt(this, property_->getDisplayName());

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setReadOnly(true);

    openButton_ = new QToolButton(this);
    openButton_->setIcon(QIcon(":/icons/open.png"));

    hLayout->addWidget(directoryLabel_);
    hLayout->addWidget(lineEdit_);
    hLayout->addWidget(openButton_);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->setStretch(1, 1);

    setLayout(hLayout);

    connect(directoryLabel_, SIGNAL(textChanged()), this, SLOT(setPropertyDisplayName()));
    connect(openButton_, SIGNAL(pressed()), this, SLOT(setPropertyValue()));
}

void DirectoryPropertyWidgetQt::setPropertyValue() {
    QString existingDir = QFileDialog::getExistingDirectory(
        this, "Open Directory", InviwoFileDialog::getPreviousPath(QString(property_->getContentType().c_str())));
    std::string dir = existingDir.toLocal8Bit().constData();

    if (!dir.empty()) {
        InviwoFileDialog::setPreviousPath(QString(property_->getContentType().c_str()), existingDir);
        setPropertyTreeInfo(dir);
        property_->set(dir);
    }
}

void DirectoryPropertyWidgetQt::updateFromProperty() {
    QDir currentDir = QDir(QString::fromStdString(property_->get()));
    lineEdit_->setText(currentDir.dirName());
    setPropertyTreeInfo(property_->get());

    lineEdit_->setDisabled(property_->getReadOnly());
    openButton_->setDisabled(property_->getReadOnly());
}

void DirectoryPropertyWidgetQt::setPropertyTreeInfo(std::string path) {
    QDir currentDir = QDir(QString::fromStdString(path));
    lineEdit_->setText(currentDir.dirName());
    QStringList files;
    QString filter = "*.*";
    files = currentDir.entryList(QStringList(filter), QDir::Files | QDir::NoSymLinks);
    std::vector<std::string> directoryTreeInfo;

    // TODO: set property tree info
    for (int i = 0; i < files.size(); i++) {
        std::string fileName = files[i].toLocal8Bit().constData();
        directoryTreeInfo.push_back(fileName);
    }

    property_->setDirectoryTree(directoryTreeInfo);
}

void DirectoryPropertyWidgetQt::setPropertyDisplayName() {
    property_->setDisplayName(directoryLabel_->getText());
}

}  // namespace
