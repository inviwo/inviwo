/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

PropertyClassIdentifier(FileProperty, "org.inviwo.FileProperty");

FileProperty::FileProperty(std::string identifier, std::string displayName, std::string value,
                           std::string contentType,
                           InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : TemplateProperty<std::string>(identifier, displayName, value, invalidationLevel, semantics)
    , acceptMode_(AcceptMode::Open)
    , fileMode_(FileMode::AnyFile)
    , contentType_(contentType) {
    addNameFilter(FileExtension("*", "All Files"));
}

FileProperty::FileProperty(const FileProperty& rhs  )
    : TemplateProperty<std::string>(rhs)
    , nameFilters_(rhs.nameFilters_)
    , acceptMode_(rhs.acceptMode_)
    , fileMode_(rhs.fileMode_) {
}

FileProperty& FileProperty::operator=(const FileProperty& that) {
    if (this != &that) {
        TemplateProperty<std::string>::operator=(that);
        nameFilters_ = that.nameFilters_;
        acceptMode_ = that.acceptMode_;
        fileMode_ = that.fileMode_;
    }
    return *this;
}

FileProperty& FileProperty::operator=(const std::string& value) {
    TemplateProperty<std::string>::operator=(value);
    return *this;
}

FileProperty* FileProperty::clone() const {
    return new FileProperty(*this);
}

FileProperty::~FileProperty() {}

void FileProperty::serialize(IvwSerializer& s) const {
    Property::serialize(s);

    std::string basePath = s.getFileName();
    std::string absoluteFilePath = get();

    if (basePath.empty())
        basePath = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_DATA);

    std::string serializePath;

    if (absoluteFilePath.size() != 0 && filesystem::sameDrive(basePath, absoluteFilePath))
        serializePath = filesystem::getRelativePath(basePath, absoluteFilePath);
    else
        serializePath = absoluteFilePath;

    s.serialize("url", serializePath);
    s.serialize("nameFilter", nameFilters_, "filter");
    s.serialize("acceptMode", acceptMode_);
    s.serialize("fileMode", fileMode_);
}

void FileProperty::deserialize(IvwDeserializer& d) {
    Property::deserialize(d);
    std::string serializePath;

    d.deserialize("url", serializePath);

    if (!filesystem::isAbsolutePath(serializePath) && !serializePath.empty()) {
        std::string basePath = d.getFileName();

        if (basePath.empty())
            basePath = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_DATA);

        basePath = filesystem::getFileDirectory(basePath);
        set(basePath + serializePath);
    } else {
        set(serializePath);
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

void FileProperty::addNameFilter(FileExtension filter) {
    nameFilters_.push_back(filter);
}

void FileProperty::clearNameFilters() {
    nameFilters_.clear(); 
}

std::vector<FileExtension> FileProperty::getNameFilters() { 
    return nameFilters_; 
}

void FileProperty::setAcceptMode(AcceptMode mode) { 
    acceptMode_ = mode; 
}

FileProperty::AcceptMode FileProperty::getAcceptMode() const {
    return acceptMode_;
}

void FileProperty::setFileMode(FileMode mode) { 
    fileMode_ = mode; 
}

FileProperty::FileMode FileProperty::getFileMode() const {
    return fileMode_; 
}

void FileProperty::setContentType(const std::string& contentType) { 
    contentType_ = contentType; 
}

std::string FileProperty::getContentType() const { 
    return contentType_; 
}

}  // namespace
