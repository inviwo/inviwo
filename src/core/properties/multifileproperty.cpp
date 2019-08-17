/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

const std::string MultiFileProperty::classIdentifier = "org.inviwo.MultiFileProperty";
std::string MultiFileProperty::getClassIdentifier() const { return classIdentifier; }

MultiFileProperty::MultiFileProperty(std::string identifier, std::string displayName,
                                     const std::vector<std::string>& value, std::string contentType,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : TemplateProperty<std::vector<std::string>>(identifier, displayName, value, invalidationLevel,
                                                 semantics)
    , acceptMode_(AcceptMode::Open)
    , fileMode_(FileMode::ExistingFiles)
    , contentType_(contentType) {
    addNameFilter(FileExtension::all());
}

MultiFileProperty::MultiFileProperty(const MultiFileProperty& rhs)
    : TemplateProperty<std::vector<std::string>>(rhs)
    , nameFilters_(rhs.nameFilters_)
    , acceptMode_(rhs.acceptMode_)
    , fileMode_(rhs.fileMode_) {}

MultiFileProperty& MultiFileProperty::operator=(const std::vector<std::string>& value) {
    TemplateProperty<std::vector<std::string>>::operator=(value);
    return *this;
}

MultiFileProperty* MultiFileProperty::clone() const { return new MultiFileProperty(*this); }

void MultiFileProperty::set(const std::string& value) {
    TemplateProperty<std::vector<std::string>>::set({filesystem::cleanupPath(value)});
}

void MultiFileProperty::set(const std::vector<std::string>& values) {
    std::vector<std::string> tmp;
    tmp.reserve(values.size());
    // clean up paths first
    for (auto elem : values) {
        tmp.push_back(filesystem::cleanupPath(elem));
    }
    TemplateProperty<std::vector<std::string>>::set(tmp);

    // figure out best matching extension based on first filename
    const auto& files = get();
    if (files.empty()) {
        setSelectedExtension(FileExtension());
    } else {
        FileExtension ext;
        FileExtension matchAll;
        for (const auto& filter : getNameFilters()) {
            if (filter.matchesAll()) {
                matchAll = filter;
            } else if (filter.matches(files.front())) {
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
}

void MultiFileProperty::set(const Property* property) {
    TemplateProperty<std::vector<std::string>>::set(property);
}

void MultiFileProperty::serialize(Serializer& s) const {
    /*
    We always use absolute paths inside of inviwo but serialize
    several version to have a higher success rate when moving stuff around.

    Saved path versions:
     1) Absolute (equivalent to this->get())
     2) Relative workspace
     3) Relative filesystem::getPath(PathType::Data)

    */
    Property::serialize(s);

    if (this->serializationMode_ == PropertySerializationMode::None) return;

    value_.serialize(s, this->serializationMode_);

    const auto workspacePath = filesystem::getFileDirectory(s.getFileName());
    const auto ivwdataPath = filesystem::getPath(PathType::Data);

    // save absolute and relative paths for each file pattern
    std::vector<std::string> workspaceRelativePaths;  // paths relative to workspace directory
    std::vector<std::string> ivwdataRelativePaths;    // paths relative to Inviwo data
    for (auto item : this->get()) {
        const auto basePath = filesystem::getFileDirectory(item);

        std::string workspaceRelative;
        std::string ivwdataRelative;
        if (!basePath.empty()) {
            if (!workspacePath.empty() && filesystem::sameDrive(workspacePath, basePath)) {
                workspaceRelative = filesystem::getRelativePath(workspacePath, basePath);
                if (workspaceRelative.empty()) {
                    workspaceRelative = ".";
                }
            }
            if (!ivwdataPath.empty() && filesystem::sameDrive(ivwdataPath, basePath)) {
                ivwdataRelative = filesystem::getRelativePath(ivwdataPath, basePath);
                if (ivwdataRelative.empty()) {
                    ivwdataRelative = ".";
                }
            }
        }
        workspaceRelativePaths.push_back(workspaceRelative);
        ivwdataRelativePaths.push_back(ivwdataRelative);
    }

    s.serialize("workspaceRelativePath", workspaceRelativePaths, "files");
    s.serialize("ivwdataRelativePath", ivwdataRelativePaths, "files");

    s.serialize("nameFilter", nameFilters_, "filter");
    s.serialize("acceptMode", acceptMode_);
    s.serialize("fileMode", fileMode_);
}

void MultiFileProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);
    bool modified = value_.deserialize(d, this->serializationMode_);

    std::vector<std::string> absolutePaths = this->get();
    std::vector<std::string> workspaceRelativePaths;  // paths relative to workspace directory
    std::vector<std::string> ivwdataRelativePaths;    // paths relative to Inviwo data

    d.deserialize("workspaceRelativePath", workspaceRelativePaths, "files");
    d.deserialize("ivwdataRelativePath", ivwdataRelativePaths, "files");

    // check whether all path lists have the same number of entries
    if ((absolutePaths.size() == workspaceRelativePaths.size()) &&
        (absolutePaths.size() == ivwdataRelativePaths.size())) {

        const auto workspacePath = filesystem::getFileDirectory(d.getFileName());
        const auto ivwdataPath = filesystem::getPath(PathType::Data);

        bool modifiedPath = false;
        for (std::size_t i = 0; i < absolutePaths.size(); ++i) {
            const auto basePath = filesystem::getFileDirectory(absolutePaths[i]);
            const auto pattern = filesystem::getFileNameWithExtension(absolutePaths[i]);

            const auto workspaceBasedPath = filesystem::cleanupPath(
                filesystem::getCanonicalPath(workspacePath + "/" + workspaceRelativePaths[i]));
            const auto ivwdataBasedPath = filesystem::cleanupPath(
                filesystem::getCanonicalPath(ivwdataPath + "/" + ivwdataRelativePaths[i]));

            if (!basePath.empty() && filesystem::fileExists(basePath)) {
                continue;
            } else if (!ivwdataRelativePaths[i].empty() &&
                       filesystem::fileExists(ivwdataBasedPath)) {
                absolutePaths[i] = ivwdataBasedPath + "/" + pattern;
                modifiedPath = true;
            } else if (!workspaceRelativePaths[i].empty() &&
                       filesystem::fileExists(workspaceBasedPath)) {
                absolutePaths[i] = workspaceBasedPath + "/" + pattern;
                modifiedPath = true;
            }
        }

        if (modifiedPath) {
            // propagate changes back to property
            this->set(absolutePaths);
            modified = true;
        }
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
    if (modified) {
        this->propertyModified();
    }
}

void MultiFileProperty::addNameFilter(std::string filter) {
    nameFilters_.push_back(FileExtension::createFileExtensionFromString(filter));
}

void MultiFileProperty::addNameFilter(FileExtension filter) { nameFilters_.push_back(filter); }

void MultiFileProperty::addNameFilters(const std::vector<FileExtension>& filters) {
    util::append(nameFilters_, filters);
}

void MultiFileProperty::clearNameFilters() { nameFilters_.clear(); }

std::vector<FileExtension> MultiFileProperty::getNameFilters() { return nameFilters_; }

void MultiFileProperty::setAcceptMode(AcceptMode mode) { acceptMode_ = mode; }

AcceptMode MultiFileProperty::getAcceptMode() const { return acceptMode_; }

void MultiFileProperty::setFileMode(FileMode mode) { fileMode_ = mode; }

FileMode MultiFileProperty::getFileMode() const { return fileMode_; }

void MultiFileProperty::setContentType(const std::string& contentType) {
    contentType_ = contentType;
}

std::string MultiFileProperty::getContentType() const { return contentType_; }

void MultiFileProperty::requestFile() {
    for (auto widget : getWidgets()) {
        if (auto filerequestable = dynamic_cast<FileRequestable*>(widget)) {
            if (filerequestable->requestFile()) return;
        }
    }
    if (getWidgets().empty()) {
        // Currently, the only difference between using the widget (Qt) and the FileDialog directly
        // is that the Qt widget remembers the previously used directory
        auto fileDialog = util::dynamic_unique_ptr_cast<FileDialog>(
            InviwoApplication::getPtr()->getDialogFactory()->create("FileDialog"));
        if (!fileDialog) {
            throw Exception(
                "Failed to create a FileDialog. Add one to the InviwoApplication::DialogFactory");
        }

        // Setup Extensions
        std::vector<FileExtension> filters = this->getNameFilters();
        fileDialog->addExtensions(filters);

        fileDialog->setCurrentFile(get().empty() ? "" : get().front());
        fileDialog->setTitle(getDisplayName());
        fileDialog->setAcceptMode(getAcceptMode());
        fileDialog->setFileMode(getFileMode());

        auto ext = getSelectedExtension();
        if (!ext.empty()) fileDialog->setSelectedExtension(ext);

        if (fileDialog->show()) {
            setSelectedExtension(fileDialog->getSelectedFileExtension());
            set(fileDialog->getSelectedFiles());
        }
    }
}

const FileExtension& MultiFileProperty::getSelectedExtension() const { return selectedExtension_; }
void MultiFileProperty::setSelectedExtension(const FileExtension& ext) { selectedExtension_ = ext; }

Document MultiFileProperty::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = TemplateProperty<std::vector<std::string>>::getDescription();
    auto table = doc.get({P("html"), P("body"), P("table", {{"identifier", "propertyInfo"}})});

    std::ostringstream stream;
    // std::copy(value_.value.begin(), value_.value.end(),
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
        case FileMode::AnyFile:
        case FileMode::ExistingFile:
        case FileMode::ExistingFiles: {
            tb(H("Files"), files);
            break;
        }

        case FileMode::Directory:
        case FileMode::DirectoryOnly: {
            tb(H("Directories"), files);
            break;
        }
    }

    tb(H("Accept Mode"), acceptMode_);
    tb(H("File Mode"), fileMode_);
    tb(H("Content Type"), contentType_);

    return doc;
}

}  // namespace inviwo
