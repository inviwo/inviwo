/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

PropertyClassIdentifier(MultiFileProperty, "org.inviwo.MultiFileProperty");

MultiFileProperty::MultiFileProperty(std::string identifier, std::string displayName, const std::vector<std::string>& value,
                           std::string contentType,
                           InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : TemplateProperty<std::vector<std::string> >(identifier, displayName, value, invalidationLevel, semantics)
    , acceptMode_(FileProperty::AcceptMode::Open)
    , fileMode_(FileProperty::FileMode::ExistingFiles)
    , contentType_(contentType) {
    addNameFilter(FileExtension("*", "All Files"));
}

MultiFileProperty::MultiFileProperty(const MultiFileProperty& rhs  )
    : TemplateProperty<std::vector<std::string> >(rhs)
    , nameFilters_(rhs.nameFilters_)
    , acceptMode_(rhs.acceptMode_)
    , fileMode_(rhs.fileMode_) {
}

MultiFileProperty& MultiFileProperty::operator=(const MultiFileProperty& that) {
    if (this != &that) {
        TemplateProperty<std::vector<std::string> >::operator=(that);
        nameFilters_ = that.nameFilters_;
        acceptMode_ = that.acceptMode_;
        fileMode_ = that.fileMode_;
    }
    return *this;
}

MultiFileProperty& MultiFileProperty::operator=(const std::vector<std::string>& value) {
    TemplateProperty<std::vector<std::string> >::operator=(value);
    return *this;
}

MultiFileProperty* MultiFileProperty::clone() const {
    return new MultiFileProperty(*this);
}

void MultiFileProperty::set(const std::string& value) {
    TemplateProperty<std::vector<std::string> >::set({ filesystem::cleanupPath(value) });
}

void MultiFileProperty::set(const std::vector<std::string>& values) {
    std::vector<std::string> tmp;
    tmp.reserve(values.size());
    // clean up paths first
    for (auto elem : values) {
        tmp.push_back(filesystem::cleanupPath(elem));
    }
    TemplateProperty<std::vector<std::string> >::set(tmp);
}

void MultiFileProperty::set(const Property *property) {
    TemplateProperty<std::vector<std::string> >::set(property);
}

void MultiFileProperty::serialize(Serializer& s) const {
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
    
    const auto absolutePaths = get();
    std::string workspaceRelativePath;
    std::string ivwdataRelativePath;

    if (!absolutePaths.empty() && !absolutePaths.front().empty()) {
        // store workspace path and relative path only for first file
        auto path = absolutePaths.front();
        auto workspacePath = filesystem::getFileDirectory(s.getFileName());
        if (!workspacePath.empty() && filesystem::sameDrive(workspacePath, path)) {
            workspaceRelativePath = filesystem::getRelativePath(workspacePath, path);
        }
        auto ivwdataPath = filesystem::getPath(PathType::Data);
        if (!ivwdataPath.empty() && filesystem::sameDrive(ivwdataPath, path)) {
            ivwdataRelativePath = filesystem::getRelativePath(ivwdataPath, path);
        }
    }

    s.serialize("absolutePath", absolutePaths, "files");
    s.serialize("workspaceRelativePath", workspaceRelativePath);
    s.serialize("ivwdataRelativePath", ivwdataRelativePath);

    s.serialize("nameFilter", nameFilters_, "filter");
    s.serialize("acceptMode", acceptMode_);
    s.serialize("fileMode", fileMode_);
}

void MultiFileProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    std::vector<std::string> absolutePaths;
    std::string workspaceRelativePath;
    std::string ivwdataRelativePath;
    std::string oldWorkspacePath;

    d.deserialize("absolutePath", absolutePaths, "files");
    d.deserialize("workspaceRelativePath", workspaceRelativePath);
    d.deserialize("ivwdataRelativePath", ivwdataRelativePath);
    d.deserialize("url", oldWorkspacePath);
    
    
    if (!oldWorkspacePath.empty()) { // fall-back if the old value "url" is used 
        if (filesystem::isAbsolutePath(oldWorkspacePath)) {
            if (absolutePaths.empty()) {   // only use url if "absolutePath" is not set
                absolutePaths.push_back(oldWorkspacePath);
            }
        }
        else {
            if (workspaceRelativePath.empty()) { // on use url if "workspaceRelativePath" is not set
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

    if (!absolutePaths.empty() && filesystem::fileExists(absolutePaths.front())) {
        set(absolutePaths);
    } else if (!workspaceRelativePath.empty() && filesystem::fileExists(workspaceBasedPath)) {
        set(workspaceBasedPath);
    } else if (!ivwdataRelativePath.empty() && filesystem::fileExists(ivwdataBasedPath)) {
        set(ivwdataBasedPath);
    } else {
        set(absolutePaths);
    }

    try {
        nameFilters_.clear();
        d.deserialize("nameFilter", nameFilters_, "filter");
        int acceptMode = static_cast<int>(acceptMode_);
        d.deserialize("acceptMode", acceptMode);
        acceptMode_ = static_cast<FileProperty::AcceptMode>(acceptMode);
        int fileMode = static_cast<int>(fileMode_);
        d.deserialize("fileMode", fileMode);
        fileMode_ = static_cast<FileProperty::FileMode>(fileMode);
    } catch (SerializationException& e) {
        LogInfo("Problem deserializing file Property: " << e.getMessage());
    }
}

void MultiFileProperty::addNameFilter(std::string filter) { 
    nameFilters_.push_back(FileExtension::createFileExtensionFromString(filter)); 
}

void MultiFileProperty::addNameFilter(FileExtension filter) {
    nameFilters_.push_back(filter);
}

void MultiFileProperty::addNameFilters(const std::vector<FileExtension>& filters) {
    util::append(nameFilters_, filters);
}

void MultiFileProperty::clearNameFilters() {
    nameFilters_.clear(); 
}

std::vector<FileExtension> MultiFileProperty::getNameFilters() { 
    return nameFilters_; 
}

void MultiFileProperty::setAcceptMode(FileProperty::AcceptMode mode) {
    acceptMode_ = mode; 
}

FileProperty::AcceptMode MultiFileProperty::getAcceptMode() const {
    return acceptMode_;
}

void MultiFileProperty::setFileMode(FileProperty::FileMode mode) {
    fileMode_ = mode; 
}

FileProperty::FileMode MultiFileProperty::getFileMode() const {
    return fileMode_; 
}

void MultiFileProperty::setContentType(const std::string& contentType) { 
    contentType_ = contentType; 
}

std::string MultiFileProperty::getContentType() const { 
    return contentType_; 
}

void MultiFileProperty::requestFile() {
    for (auto widget : getWidgets()) {
        if (auto filerequestable = dynamic_cast<FileRequestable*>(widget)) {
            if(filerequestable->requestFile()) return;
        }
    }
}

Document MultiFileProperty::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = TemplateProperty<std::vector<std::string> >::getDescription();
    auto table = doc.get({P("html"), P("body"), P("table", {{"identifier", "propertyInfo"}})});


    std::ostringstream stream;
    //std::copy(value_.value.begin(), value_.value.end(),
    //          std::ostream_iterator<std::string>(stream, "<br/>"));
    std::string currentPath = "";
    // compile compact list of selected files, binning all files of the same directory
    for (auto elem : value_.value) {
        auto dir = filesystem::getFileDirectory(elem);
        auto filename = filesystem::getFileNameWithExtension(elem);
        if (dir != currentPath) {
            currentPath = dir;
            stream << "<b>" << dir << "</b><br/>";
        }
        stream << filename << "<br/>";
    }
    std::string files = stream.str();

    utildoc::TableBuilder tb(table);
    switch (fileMode_) {
        case FileProperty::FileMode::AnyFile:
        case FileProperty::FileMode::ExistingFile:
        case FileProperty::FileMode::ExistingFiles: {
            tb(H("Files"), files);
            break;
        }

        case FileProperty::FileMode::Directory:
        case FileProperty::FileMode::DirectoryOnly: {
            tb(H("Directories"), files);
            break;
        }
    }

    tb(H("Accept Mode"), acceptMode_);
    tb(H("File Mode"), fileMode_);
    tb(H("Content Type"), contentType_);

    return doc;
}

}  // namespace
