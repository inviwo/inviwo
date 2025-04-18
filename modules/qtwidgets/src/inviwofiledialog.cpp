/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/qtwidgets/inviwofiledialog.h>

#include <inviwo/core/util/filedialogstate.h>  // for FileMode, AcceptMode, AcceptMode::Open
#include <inviwo/core/util/fileextension.h>    // for FileExtension
#include <inviwo/core/util/filesystem.h>       // for fileExists, directoryExists, findBasePath
#include <inviwo/core/util/logcentral.h>       // for LogCentral
#include <inviwo/core/util/pathtype.h>         // for PathType
#include <modules/qtwidgets/inviwoqtutils.h>   // for toQString, fromQString

#include <ostream>  // for operator<<
#include <utility>  // for pair

#include <QDialog>         // for QDialog, QDialog::Accepted
#include <QDir>            // for QDir
#include <QSettings>       // for QSettings
#include <QStandardPaths>  // for QStandardPaths, QStandardPaths::DesktopLoc...
#include <QVariant>        // for QVariant

class QWidget;

namespace inviwo {

InviwoFileDialog::InviwoFileDialog(QWidget* parent, std::string_view title,
                                   std::string_view pathType, const std::filesystem::path& path)
    : QFileDialog(parent, utilqt::toQString(title))
    , pathType_(utilqt::toQString(pathType))
    , currentPath_() {
    setObjectName("InviwoFileDialog");
    setCurrentFile(path);
    sidebarURLs_ << QUrl::fromLocalFile(
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    sidebarURLs_ << QUrl::fromLocalFile(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    QFileDialog::setOption(QFileDialog::DontUseNativeDialog, false);
    QFileDialog::setOption(QFileDialog::DontResolveSymlinks, true);
    QObject::connect(this, SIGNAL(filterSelected(const QString&)), this,
                     SLOT(filterSelectionChanged(const QString&)));
}

bool InviwoFileDialog::show() { return this->exec() == QDialog::Accepted; }

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
        auto filter = utilqt::toQString(selectedFilter_.toString());
        if (extensions_.contains(filter)) {
            QFileDialog::selectNameFilter(filter);
        }
    }

    if (!currentPath_.isEmpty() && QDir(currentPath_).exists()) {
        QFileDialog::setDirectory(currentPath_);
    }

    int ret = QFileDialog::exec();
    if (ret == QDialog::Accepted) {
        setPreviousPath(pathType_, directory().absolutePath());
        setPreviousExtension(pathType_, selectedNameFilter());
    }

    return ret;
}

void InviwoFileDialog::setTitle(std::string_view title) {
    this->setWindowTitle(utilqt::toQString(title));
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
            QFileDialog::setOption(QFileDialog::ShowDirsOnly, false);
            break;
        case inviwo::FileMode::ExistingFiles:
            QFileDialog::setFileMode(QFileDialog::ExistingFiles);
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
        default:
            return inviwo::FileMode::AnyFile;
    }
}
void InviwoFileDialog::setContentType(std::string_view contentType) {
    pathType_ = utilqt::toQString(contentType);
}

std::string InviwoFileDialog::getContentType() const { return utilqt::fromQString(pathType_); }

void InviwoFileDialog::setCurrentDirectory(const std::filesystem::path& path) {
    if (!path.empty()) {
        currentPath_ = utilqt::toQString(path);
    } else {
        // use default path based on pathType
        currentPath_ = getPreviousPath(pathType_);
    }
    // use the full path, regardless of it potentially containing a file name
    // This will handle the case correctly when the path only contains directories.
    QFileDialog::setDirectory(currentPath_);

    // if the given path points to a file, select it when the dialog is opened
    if (path.has_filename()) {
        if (QFile::exists(currentPath_)) {
            QFileDialog::selectFile(currentPath_);
        }
    }
}

void InviwoFileDialog::setCurrentFile(const std::filesystem::path& filename) {
    std::filesystem::path path;
    if (!filename.empty()) {
        if (std::filesystem::is_directory(filename)) {
            // given file name is a path
            path = filename;
        } else {
            // if a file is selected, extract the folder path for the dialog
            path = filename.parent_path();
        }
    }

    setCurrentDirectory(path);
    if (std::filesystem::is_regular_file(filename)) {
        QFileDialog::selectFile(utilqt::toQString(filename));
    }
}

std::vector<std::filesystem::path> InviwoFileDialog::getSelectedFiles() const {
    std::vector<std::filesystem::path> filenames;
    for (auto file : QFileDialog::selectedFiles()) {
        filenames.emplace_back(utilqt::toPath(file));
    }
    return filenames;
}

void InviwoFileDialog::addExtension(const FileExtension& fileExt) {
    // create Qt compatible string
    std::string str{fileExt.toString()};

    // add entry to the extension map
    auto retval = extmap_.emplace(str, fileExt);
    if (retval.second) {  // insert successful
        extensions_ << utilqt::toQString(str);
    } else {
        log::error("Extension already registered: {}", str);
    }
}

void InviwoFileDialog::addExtension(std::string_view ext, std::string_view description) {
    FileExtension fileExt{ext, description};
    addExtension(fileExt);
}

void InviwoFileDialog::addExtension(std::string_view extString) {
    FileExtension fileExt{FileExtension::createFileExtensionFromString(extString)};
    addExtension(fileExt);
}

void InviwoFileDialog::addExtensions(const std::vector<FileExtension>& extensions) {
    for (auto ext : extensions) {
        addExtension(ext);
    }
}

void InviwoFileDialog::filterSelectionChanged(const QString& filter) {
    // try to find matching filter in extension map
    selectedFilter_ = getMatchingFileExtension(filter);
}

FileExtension InviwoFileDialog::getSelectedFileExtension() const { return selectedFilter_; }

void InviwoFileDialog::setSelectedExtension(const FileExtension& ext) { selectedFilter_ = ext; }

FileExtension InviwoFileDialog::getMatchingFileExtension(const QString& extStr) {
    // try to find matching filter in extension map
    auto it = extmap_.find(utilqt::fromQString(extStr));
    if (it != extmap_.end()) {
        return it->second;
    } else {
        return FileExtension::createFileExtensionFromString(utilqt::fromQString(extStr));
    }
}

void InviwoFileDialog::addSidebarPath(const PathType& path) {
    addSidebarPath(filesystem::getPath(path));
}

void InviwoFileDialog::addSidebarPath(const std::filesystem::path& path) {
    addSidebarPath(utilqt::toQString(std::filesystem::absolute(path)));
}

void InviwoFileDialog::addSidebarPath(const QString& path) {
    sidebarURLs_ << QUrl::fromLocalFile(QDir(path).absolutePath());
}

QString InviwoFileDialog::getPreviousPath(const QString& pathType) {
    QSettings settings;
    settings.beginGroup("InviwoFileDialog");

    QString defaultPath;
    if (pathType != "default") {
        defaultPath = getPreviousPath("default");
    } else {
        defaultPath = utilqt::toQString(filesystem::findBasePath());
        if (!QDir(defaultPath).exists()) {
            defaultPath.clear();
        }
    }

    const QVariant& variant = settings.value(pathType, defaultPath);
    settings.endGroup();
    return variant.toString();
}

void InviwoFileDialog::setPreviousPath(const QString& pathType, const QString& path) {
    QSettings settings;
    settings.beginGroup("InviwoFileDialog");
    settings.setValue(pathType, path);
    settings.endGroup();
}

QString InviwoFileDialog::getPreviousExtension(const QString& pathType) {
    QSettings settings;
    settings.beginGroup("InviwoFileDialog");
    QString setting = pathType + "_extension";

    QString defaultExt;
    if (pathType != "default") {
        defaultExt = getPreviousExtension("default");
    } else {
        defaultExt = "";
    }

    const QVariant& variant = settings.value(setting, defaultExt);
    settings.endGroup();
    return variant.toString();
}

void InviwoFileDialog::setPreviousExtension(const QString& pathType, const QString& path) {
    QSettings settings;
    settings.beginGroup("InviwoFileDialog");
    QString setting = pathType + "_extension";
    settings.setValue(setting, path);
    settings.endGroup();
}

}  // namespace inviwo
