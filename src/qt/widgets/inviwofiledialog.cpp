/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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
#include <inviwo/core/util/filesystem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QStandardPaths>
#include <QDir>
#include <warn/pop>

namespace inviwo {

InviwoFileDialog::InviwoFileDialog(QWidget *parent, const std::string &title,
                                   const std::string &pathType, const std::string &path)
    : QFileDialog(parent, QString::fromStdString(title))
    , pathType_(QString::fromStdString(pathType))
    , currentPath_() {
    setCurrentFile(path);
    sidebarURLs_ << QUrl::fromLocalFile(
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    sidebarURLs_ << QUrl::fromLocalFile(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    useNativeDialog();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    QFileDialog::setOption(QFileDialog::DontUseCustomDirectoryIcons);
#endif

    QObject::connect(this, SIGNAL(filterSelected(const QString &)), this,
                     SLOT(filterSelectionChanged(const QString &)));
}

bool InviwoFileDialog::show() {
    return this->exec() == QDialog::Accepted;
}

int InviwoFileDialog::exec() {
    QFileDialog::setNameFilters(extensions_);
    QFileDialog::setSidebarUrls(sidebarURLs_);
    // use filter used for this path type last time
    if (selectedFilter_.empty()) {
        QString filter{getPreviousExtension(pathType_)};

        if (extensions_.contains(filter)) {
            QFileDialog::selectNameFilter(filter);
        }
        // initialize selected filter
        selectedFilter_ = getMatchingFileExtension(filter);
    } else {
        auto filter = QString::fromStdString(selectedFilter_.toString());
        if (extensions_.contains(filter)) {
            QFileDialog::selectNameFilter(filter);
        }
    }

    if (!currentPath_.isEmpty()) {
        QFileDialog::setDirectory(currentPath_);
    }

    int ret = QFileDialog::exec();
    if (ret == QDialog::Accepted) {
        setPreviousPath(pathType_, directory().absolutePath());
        setPreviousExtension(pathType_, selectedNameFilter());
    }

    return ret;
}

void InviwoFileDialog::setTitle(const std::string & title) {
    this->setWindowTitle(QString::fromStdString(title));
}

void InviwoFileDialog::setAcceptMode(inviwo::AcceptMode mode) {
    switch (mode) {
        case inviwo::AcceptMode::Save:
            QFileDialog::setAcceptMode(QFileDialog::AcceptSave);
            break;
        case inviwo::AcceptMode::Open:
            QFileDialog::setAcceptMode(QFileDialog::AcceptOpen);
            break;
        default:
            QFileDialog::setAcceptMode(QFileDialog::AcceptOpen);
            break;
    }
}

AcceptMode InviwoFileDialog::getAcceptMode() const {
    switch (QFileDialog::acceptMode()) {
        case QFileDialog::AcceptSave:
            return inviwo::AcceptMode::Save;
        case QFileDialog::AcceptOpen:
            return inviwo::AcceptMode::Open;
        default:
            return inviwo::AcceptMode::Open;
    }
}

void InviwoFileDialog::setFileMode(inviwo::FileMode mode) {
    switch (mode) {
        case inviwo::FileMode::AnyFile:
            QFileDialog::setFileMode(QFileDialog::AnyFile);
            break;
        case inviwo::FileMode::ExistingFile:
            QFileDialog::setFileMode(QFileDialog::ExistingFile);
            break;
        case inviwo::FileMode::Directory:
            QFileDialog::setFileMode(QFileDialog::Directory);
            break;
        case inviwo::FileMode::ExistingFiles:
            QFileDialog::setFileMode(QFileDialog::ExistingFiles);
            break;
        case inviwo::FileMode::DirectoryOnly:
            QFileDialog::setFileMode(QFileDialog::Directory);
            QFileDialog::setOption(QFileDialog::ShowDirsOnly);
            break;
        default:
            QFileDialog::setFileMode(QFileDialog::AnyFile);
            break;
    }
}

FileMode InviwoFileDialog::getFileMode() const {
    switch (QFileDialog::fileMode()) {
        case FileMode::AnyFile:
            return inviwo::FileMode::AnyFile;
        case FileMode::ExistingFile:
            return inviwo::FileMode::ExistingFile;
        case FileMode::Directory:
            return inviwo::FileMode::Directory;
        case FileMode::ExistingFiles:
            return inviwo::FileMode::ExistingFiles;
        case FileMode::DirectoryOnly:
            return inviwo::FileMode::DirectoryOnly;
        default:
            return inviwo::FileMode::AnyFile;
    }
}
void InviwoFileDialog::useNativeDialog(const bool &use) {
    QFileDialog::setOption(QFileDialog::DontUseNativeDialog, !use);
}

void InviwoFileDialog::setCurrentDirectory(const std::string &path) {
    std::string fileName;

    if (!path.empty()) {
        currentPath_ = QString::fromStdString(path);
    }
    else {
        // use default path based on pathType
        currentPath_ = getPreviousPath(pathType_);
    }
    // use the full path, regardless of it potentially containing a file name
    // This will handle the case correctly when the path only contains directories.
    QFileDialog::setDirectory(currentPath_);

    // if the given path points to a file, select it when the dialog is opened
    if (!fileName.empty()) {
        //auto filePath = currentPath_.toStdString() + '/' + fileName;
        if (filesystem::fileExists(currentPath_.toStdString())) {
            QFileDialog::selectFile(currentPath_);
        }
    }
}

void InviwoFileDialog::setCurrentFile(const std::string &filename) {
    std::string path;
    bool fileExists = false;
    if (!filename.empty()) {
        if (filesystem::directoryExists(filename)) {
            // given file name is a path
            path = filename;
        }
        else {
            // if a file is selected, extract the folder path for the dialog
            path = filesystem::getFileDirectory(filename);
            fileExists = filesystem::fileExists(filename);
        }
    }

    setCurrentDirectory(path);
    if (fileExists) {
        QFileDialog::selectFile(QString::fromStdString(filename));
    }
}

std::vector<std::string> InviwoFileDialog::getSelectedFiles() const {
    std::vector<std::string> filenames;
    for (auto file : QFileDialog::selectedFiles()) {
        filenames.push_back(file.toStdString());
    }
    return filenames;
}

void InviwoFileDialog::addExtension(const FileExtension &fileExt) {
    // create Qt compatible string
    std::string str{ fileExt.toString() };

    // add entry to the extension map
    auto retval = extmap_.emplace(str, fileExt);
    if (retval.second) { // insert successful
        extensions_ << QString::fromStdString(str);
    }
    else {
        LogError("Extension already registered: " << str);
    }
}

void InviwoFileDialog::addExtension(const std::string &ext, const std::string &description) {
    FileExtension fileExt{ ext, description };
    addExtension(fileExt);
}

void InviwoFileDialog::addExtension(const std::string &extString) {
    FileExtension fileExt{ FileExtension::createFileExtensionFromString(extString) };
    addExtension(fileExt);
}

void InviwoFileDialog::addExtensions(const std::vector<FileExtension>& extensions) {
    for (auto ext : extensions) {
        addExtension(ext);
    }
}

void InviwoFileDialog::filterSelectionChanged(const QString &filter) {
    // try to find matching filter in extension map
    selectedFilter_ = getMatchingFileExtension(filter);
}

FileExtension InviwoFileDialog::getSelectedFileExtension() const { return selectedFilter_; }

void InviwoFileDialog::setSelectedExtenstion(const FileExtension& ext) {
    selectedFilter_ = ext;
}

FileExtension InviwoFileDialog::getMatchingFileExtension(const QString &extStr) {
    // try to find matching filter in extension map
    auto it = extmap_.find(extStr.toStdString());
    if (it != extmap_.end()) {
        return it->second;
    }
    else {
        return FileExtension::createFileExtensionFromString(extStr.toStdString());
    }
}

void InviwoFileDialog::addSidebarPath(const PathType &path) {
    addSidebarPath(filesystem::getPath(path));
}

void InviwoFileDialog::addSidebarPath(const std::string &path) {
    sidebarURLs_ << QUrl::fromLocalFile(QDir(path.c_str()).absolutePath());
}

void InviwoFileDialog::addSidebarPath(const QString &path) {
    sidebarURLs_ << QUrl::fromLocalFile(QDir(path).absolutePath());
}

QSettings InviwoFileDialog::globalSettings_("Inviwo", "Inviwo");

QString InviwoFileDialog::getPreviousPath(const QString &pathType) {
    globalSettings_.beginGroup("InviwoFileDialog");

    QString defaultPath;
    if (pathType != "default") {
        defaultPath = getPreviousPath("default");
    } else {
        defaultPath = QString::fromStdString(InviwoApplication::getPtr()->getBasePath());
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
    } else {
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