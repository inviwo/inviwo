/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

const std::string FileProperty::classIdentifier = "org.inviwo.FileProperty";
std::string FileProperty::getClassIdentifier() const { return classIdentifier; }

FileProperty::FileProperty(std::string identifier, std::string displayName, std::string value,
                           std::string contentType, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : TemplateProperty<std::string>(identifier, displayName, value, invalidationLevel, semantics)
    , acceptMode_(AcceptMode::Open)
    , fileMode_(FileMode::AnyFile)
    , contentType_(contentType) {
    addNameFilter(FileExtension::all());
}

FileProperty& FileProperty::operator=(const std::string& value) {
    TemplateProperty<std::string>::operator=(value);
    return *this;
}

FileProperty* FileProperty::clone() const { return new FileProperty(*this); }

void FileProperty::set(const std::string& value) {
    TemplateProperty<std::string>::set(filesystem::cleanupPath(value));

    // figure out best matching extension based on given path
    FileExtension ext;
    FileExtension matchAll;
    for (const auto& filter : getNameFilters()) {
        if (filter.matchesAll()) {
            matchAll = filter;
        } else if (filter.matches(get())) {
            ext = filter;
            break;
        }
    }
    if (ext.empty() && !matchAll.empty()) {
        setSelectedExtension(matchAll);
    } else {
        setSelectedExtension(ext);
    }
}

void FileProperty::set(const Property* property) { TemplateProperty<std::string>::set(property); }

void FileProperty::serialize(Serializer& s) const {
    /*
    We always use absolute paths inside of inviwo but serialize
    several version to have a higher success rate when moving stuff around.

    Saved path versions:
     1) Absolute
     2) Relative workspace
     3) Relative filesystem::getPath(PathType::Data)
    */
    Property::serialize(s);

    if (this->serializationMode_ == PropertySerializationMode::None) return;

    const std::string absolutePath = get();
    std::string workspaceRelativePath;
    std::string ivwdataRelativePath;

    if (!absolutePath.empty()) {
        auto workspacePath = filesystem::getFileDirectory(s.getFileName());
        if (!workspacePath.empty() && filesystem::sameDrive(workspacePath, absolutePath)) {
            workspaceRelativePath = filesystem::getRelativePath(workspacePath, absolutePath);
        }
        auto ivwdataPath = filesystem::getPath(PathType::Data);
        if (!ivwdataPath.empty() && filesystem::sameDrive(ivwdataPath, absolutePath)) {
            ivwdataRelativePath = filesystem::getRelativePath(ivwdataPath, absolutePath);
        }
    }

    s.serialize("absolutePath", absolutePath);
    s.serialize("workspaceRelativePath", workspaceRelativePath);
    s.serialize("ivwdataRelativePath", ivwdataRelativePath);

    s.serialize("nameFilter", nameFilters_, "filter");
    s.serialize("acceptMode", acceptMode_);
    s.serialize("fileMode", fileMode_);
}

void FileProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    std::string absolutePath;
    std::string workspaceRelativePath;
    std::string ivwdataRelativePath;
    std::string oldWorkspacePath;

    d.deserialize("absolutePath", absolutePath);
    d.deserialize("workspaceRelativePath", workspaceRelativePath);
    d.deserialize("ivwdataRelativePath", ivwdataRelativePath);
    d.deserialize("url", oldWorkspacePath);

    if (!oldWorkspacePath.empty()) {  // fallback if the old value "url" is used
        if (filesystem::isAbsolutePath(oldWorkspacePath)) {
            if (absolutePath.empty()) {  // on use url if "absolutePath" is not set
                absolutePath = oldWorkspacePath;
            }
        } else {
            if (workspaceRelativePath
                    .empty()) {  // on use url if "workspaceRelativePath" is not set
                workspaceRelativePath = oldWorkspacePath;
            }
        }
    }

    const auto workspacePath = filesystem::getFileDirectory(d.getFileName());
    const auto ivwdataPath = filesystem::getPath(PathType::Data);

    const auto workspaceBasedPath =
        filesystem::getCanonicalPath(workspacePath + "/" + workspaceRelativePath);
    const auto ivwdataBasedPath =
        filesystem::getCanonicalPath(ivwdataPath + "/" + ivwdataRelativePath);

    // Prefer the relative paths to make relocation easier.
    if (!ivwdataRelativePath.empty() && filesystem::fileExists(ivwdataBasedPath)) {
        set(ivwdataBasedPath);
    } else if (!workspaceRelativePath.empty() && filesystem::fileExists(workspaceBasedPath)) {
        set(workspaceBasedPath);
    } else {
        set(absolutePath);
    }

    try {
        nameFilters_.clear();
        d.deserialize("nameFilter", nameFilters_, "filter");
        int acceptMode = static_cast<int>(acceptMode_);
        d.deserialize("acceptMode", acceptMode);
        acceptMode_ = static_cast<AcceptMode>(acceptMode);
        int fileMode = static_cast<int>(fileMode_);
        d.deserialize("fileMode", fileMode);
        fileMode_ = static_cast<FileMode>(fileMode);
    } catch (SerializationException& e) {
        LogInfo("Problem deserializing file Property: " << e.getMessage());
    }
}

void FileProperty::addNameFilter(std::string filter) {
    nameFilters_.push_back(FileExtension::createFileExtensionFromString(filter));
}

void FileProperty::addNameFilter(FileExtension filter) { nameFilters_.push_back(filter); }

void FileProperty::addNameFilters(const std::vector<FileExtension>& filters) {
    util::append(nameFilters_, filters);
}

void FileProperty::clearNameFilters() { nameFilters_.clear(); }

std::vector<FileExtension> FileProperty::getNameFilters() { return nameFilters_; }

void FileProperty::setAcceptMode(AcceptMode mode) { acceptMode_ = mode; }

AcceptMode FileProperty::getAcceptMode() const { return acceptMode_; }

void FileProperty::setFileMode(FileMode mode) { fileMode_ = mode; }

FileMode FileProperty::getFileMode() const { return fileMode_; }

void FileProperty::setContentType(const std::string& contentType) { contentType_ = contentType; }

std::string FileProperty::getContentType() const { return contentType_; }

void FileProperty::requestFile() {
    for (auto widget : getWidgets()) {
        if (auto filerequestable = dynamic_cast<FileRequestable*>(widget)) {
            if (filerequestable->requestFile()) return;
        }
    }
}

const FileExtension& FileProperty::getSelectedExtension() const { return selectedExtension_; }
void FileProperty::setSelectedExtension(const FileExtension& ext) { selectedExtension_ = ext; }

Document FileProperty::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = TemplateProperty<std::string>::getDescription();
    auto table = doc.get({P("html"), P("body"), P("table", {{"identifier", "propertyInfo"}})});

    utildoc::TableBuilder tb(table);
    switch (fileMode_) {
        case FileMode::AnyFile:
        case FileMode::ExistingFile:
        case FileMode::ExistingFiles: {
            tb(H("File"), value_.value);
            break;
        }

        case FileMode::Directory:
        case FileMode::DirectoryOnly: {
            tb(H("Directory"), value_.value);
            break;
        }
    }

    tb(H("Accept Mode"), acceptMode_);
    tb(H("File Mode"), fileMode_);
    tb(H("Content Type"), contentType_);

    return doc;
}

}  // namespace inviwo
