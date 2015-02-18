/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/inviwofiledialog.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

namespace inviwo {

InviwoFileDialog::InviwoFileDialog(QWidget *parent, const std::string &title,
                                   const std::string &pathType)
    : QFileDialog(parent, title.c_str(), 
      getPreviousPath(QString(pathType.c_str())))
    , pathType_(QString(pathType.c_str())) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    sidebarURLs_ << QUrl::fromLocalFile(
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    sidebarURLs_ << QUrl::fromLocalFile(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
#else
    sidebarURLs_ << QUrl::fromLocalFile(
        QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    sidebarURLs_ << QUrl::fromLocalFile(
        QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
#endif
    useNativeDialog();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    QFileDialog::setOption(QFileDialog::DontUseCustomDirectoryIcons);
#endif
}

void InviwoFileDialog::useNativeDialog(const bool &use) {
    QFileDialog::setOption(QFileDialog::DontUseNativeDialog, !use);
}

void InviwoFileDialog::setNameFilter(const QString &filter) {
    LogWarn(
        "Use of QT function setNameFilter should not be used on InviwoFileDialog: use addExtension "
        "instead");
    QFileDialog::setNameFilter(filter);
}

void InviwoFileDialog::setNameFilters(const QStringList &filters) {
    LogWarn(
        "Use of QT function setNameFilters should not be used on InviwoFileDialog: use "
        "addExtension instead");
    QFileDialog::setNameFilters(filters);
}

void InviwoFileDialog::setSidebarUrls(const QList<QUrl> &urls) {
    LogWarn(
        "Use of QT function setSidebarUrls should not be used on InviwoFileDialog: use "
        "addSidebarPath instead");
    QFileDialog::setSidebarUrls(urls);
}

void InviwoFileDialog::addExtension(const std::string &ext, const std::string &description) {
    std::stringstream ss;
    ss << description << "(*." << ext << ")";
    addExtension(ss.str());
}

void InviwoFileDialog::addExtension(const std::string &extString) {
    extension_ << extString.c_str();
}

void InviwoFileDialog::addSidebarPath(const InviwoApplication::PathType &path) {
    addSidebarPath(InviwoApplication::getPtr()->getPath(path));
}

void InviwoFileDialog::addSidebarPath(const std::string &path) {
    sidebarURLs_ << QUrl::fromLocalFile(QDir(path.c_str()).absolutePath());
}

void InviwoFileDialog::addSidebarPath(const QString &path) {
    sidebarURLs_ << QUrl::fromLocalFile(QDir(path).absolutePath());
}

int InviwoFileDialog::exec() {
    QFileDialog::setNameFilters(extension_);
    QFileDialog::setSidebarUrls(sidebarURLs_);
    QFileDialog::selectNameFilter(getPreviousExtension(pathType_));

    int ret = QFileDialog::exec();
    if (ret == QDialog::Accepted) {
        setPreviousPath(pathType_, directory().absolutePath());
        setPreviousExtension(pathType_, selectedNameFilter());
        

    }

    return ret;
}

QSettings InviwoFileDialog::globalSettings_("Inviwo", "Inviwo");

QString InviwoFileDialog::getPreviousPath(const QString &pathType) {
    globalSettings_.beginGroup("InviwoFileDialog");

    QString defaultPath;
    if (pathType != "default") {
        defaultPath = getPreviousPath("default");
    }
    else {
        defaultPath = QString(InviwoApplication::getPtr()->getBasePath().c_str());
    }

    const QVariant &variant = globalSettings_.value(pathType, defaultPath);
    globalSettings_.endGroup();
    return variant.toString();
}

void InviwoFileDialog::setPreviousPath(const QString &pathType, const QString &path) {
    globalSettings_.beginGroup("InviwoFileDialog");
    globalSettings_.setValue(pathType, path);
    globalSettings_.endGroup();
}

QString InviwoFileDialog::getPreviousExtension(const QString &pathType) {
    globalSettings_.beginGroup("InviwoFileDialog");
    QString setting = pathType + "_extension";

    QString defaultExt;
    if (pathType != "default") {
        defaultExt = getPreviousExtension("default");
    }
    else {
        defaultExt = "";
    }

    const QVariant &variant = globalSettings_.value(setting, defaultExt);
    globalSettings_.endGroup();
    return variant.toString();
}

void InviwoFileDialog::setPreviousExtension(const QString &pathType, const QString &path) {
    globalSettings_.beginGroup("InviwoFileDialog");
    QString setting = pathType + "_extension";
    globalSettings_.setValue(setting, path);
    globalSettings_.endGroup();
}
}