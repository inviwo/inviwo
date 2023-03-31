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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

const std::string FileProperty::classIdentifier = "org.inviwo.FileProperty";
std::string FileProperty::getClassIdentifier() const { return classIdentifier; }

FileProperty::FileProperty(std::string_view identifier, std::string_view displayName, Document help,
                           const std::filesystem::path& value, std::string_view contentType,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TemplateProperty<std::filesystem::path>(identifier, displayName, std::move(help), std::string{},
                                              invalidationLevel, semantics)
    , acceptMode_(AcceptMode::Open)
    , fileMode_(FileMode::AnyFile)
    , contentType_(contentType) {
    // Explicitly set the file name here since the TemplateProperty itself is initialized with an
    // empty string as default value. This ensures that the set file name is always serialized.
    set(std::string{value});
    addNameFilter(FileExtension::all());
}

FileProperty::FileProperty(std::string_view identifier, std::string_view displayName,
                           const std::filesystem::path& value, std::string_view contentType,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : FileProperty(identifier, displayName, Document{}, value, contentType, invalidationLevel,
                   semantics) {}

FileProperty& FileProperty::operator=(const std::filesystem::path& value) {
    TemplateProperty<std::filesystem::path>::operator=(value);
    return *this;
}

FileProperty* FileProperty::clone() const { return new FileProperty(*this); }

void FileProperty::set(const std::filesystem::path& value) {
    bool modified = false;
    if (value_ != value) {
        value_ = value;
        modified |= true;
    }

    if (!selectedExtension_.matches(value_.value)) {
        const auto it = std::find_if(nameFilters_.begin(), nameFilters_.end(), [&](const auto& f) {
            return !f.matchesAll() && f.matches(value_.value);
        });
        if (it != nameFilters_.end() && selectedExtension_ != *it) {
            selectedExtension_ = *it;
            modified |= true;

        } else {
            const auto allit = std::find_if(nameFilters_.begin(), nameFilters_.end(),
                                            [&](const auto& f) { return f.matchesAll(); });
            if (allit != nameFilters_.end() && selectedExtension_ != *allit) {
                selectedExtension_ = *allit;
                modified |= true;
            }
        }
    }

    if (modified) propertyModified();
}

void FileProperty::set(const std::filesystem::path& value, const FileExtension& selectedExtension) {
    bool modified = false;
    if (value_ != value) {
        value_ = value;
        modified |= true;
    }
    if (selectedExtension_ != selectedExtension) {
        selectedExtension_ = selectedExtension;
        modified |= true;
    }
    if (modified) propertyModified();
}

void FileProperty::set(const Property* property) {
    if (auto prop = dynamic_cast<const FileProperty*>(property)) {
        set(prop->get(), prop->getSelectedExtension());
    } else {
        TemplateProperty<std::filesystem::path>::set(property);
    }
}

FileProperty::operator const std::filesystem::path&() const { return this->value_.value; }

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

    const std::filesystem::path absolutePath = get();
    std::filesystem::path workspaceRelativePath;
    std::filesystem::path ivwdataRelativePath;

    if (!absolutePath.empty()) {
        auto workspacePath = s.getFileName().parent_path();
        if (!workspacePath.empty() && workspacePath.root_name() == absolutePath.root_name()) {
            workspaceRelativePath = std::filesystem::relative(absolutePath, workspacePath);
        }
        auto ivwdataPath = filesystem::getPath(PathType::Data);
        if (!ivwdataPath.empty() && ivwdataPath.root_name() == absolutePath.root_name()) {
            ivwdataRelativePath = std::filesystem::relative(absolutePath, ivwdataPath);
        }
    }

    s.serialize("absolutePath", absolutePath);
    s.serialize("workspaceRelativePath", workspaceRelativePath);
    s.serialize("ivwdataRelativePath", ivwdataRelativePath);

    // We don't serialize the filename Filter since they are usually a runtime thing depending
    // on which readers/writers that a available
    s.serialize("selectedExtension", selectedExtension_);
    s.serialize("acceptMode", acceptMode_);
    s.serialize("fileMode", fileMode_);
}

void FileProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    std::filesystem::path absolutePath;
    std::filesystem::path workspaceRelativePath;
    std::filesystem::path ivwdataRelativePath;
    std::filesystem::path oldWorkspacePath;

    d.deserialize("absolutePath", absolutePath);
    d.deserialize("workspaceRelativePath", workspaceRelativePath);
    d.deserialize("ivwdataRelativePath", ivwdataRelativePath);
    d.deserialize("url", oldWorkspacePath);

    if (!oldWorkspacePath.empty()) {  // fallback if the old value "url" is used
        if (oldWorkspacePath.is_absolute()) {
            if (absolutePath.empty()) {  // on use url if "absolutePath" is not set
                absolutePath = oldWorkspacePath;
            }
        } else {
            // on use url if "workspaceRelativePath" is not set
            if (workspaceRelativePath.empty()) {
                workspaceRelativePath = oldWorkspacePath;
            }
        }
    }

    const auto workspacePath = d.getFileName().parent_path();
    const auto ivwdataPath = filesystem::getPath(PathType::Data);

    const auto workspaceBasedPath =
        std::filesystem::weakly_canonical(workspacePath / workspaceRelativePath);
    const auto ivwdataBasedPath =
        std::filesystem::weakly_canonical(ivwdataPath / ivwdataRelativePath);

    // Prefer the relative paths to make relocation easier.
    if (!ivwdataRelativePath.empty() && std::filesystem::is_regular_file(ivwdataBasedPath)) {
        set(ivwdataBasedPath);
    } else if (!workspaceRelativePath.empty() &&
               std::filesystem::is_regular_file(workspaceBasedPath)) {
        set(workspaceBasedPath);
    } else {
        set(absolutePath);
    }

    try {
        d.deserialize("selectedExtension", selectedExtension_);
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

void FileProperty::addNameFilter(std::string_view filter) {
    nameFilters_.push_back(FileExtension::createFileExtensionFromString(filter));
}

void FileProperty::addNameFilter(FileExtension filter) { nameFilters_.push_back(filter); }

void FileProperty::addNameFilters(const std::vector<FileExtension>& filters) {
    util::append(nameFilters_, filters);
}

void FileProperty::clearNameFilters() { nameFilters_.clear(); }

const std::vector<FileExtension>& FileProperty::getNameFilters() const { return nameFilters_; }

void FileProperty::setAcceptMode(AcceptMode mode) { acceptMode_ = mode; }

AcceptMode FileProperty::getAcceptMode() const { return acceptMode_; }

void FileProperty::setFileMode(FileMode mode) { fileMode_ = mode; }

FileMode FileProperty::getFileMode() const { return fileMode_; }

void FileProperty::setContentType(std::string_view contentType) { contentType_ = contentType; }

const std::string& FileProperty::getContentType() const { return contentType_; }

void FileProperty::requestFile() {
    for (auto widget : getWidgets()) {
        if (auto filerequestable = dynamic_cast<FileRequestable*>(widget)) {
            filerequestable->requestFile();
            return;
        }
    }
    // No FileRequestable widget found, use the factory.
    // Currently, the only difference between using the widget (Qt) and the FileDialog directly
    // is that the Qt widget stores the previously used directory
    auto fileDialog = util::dynamic_unique_ptr_cast<FileDialog>(
        InviwoApplication::getPtr()->getDialogFactory()->create("FileDialog"));
    if (!fileDialog) {
        throw Exception(
            "Failed to create a FileDialog. Add one to the InviwoApplication::DialogFactory",
            IVW_CONTEXT);
    }

    fileDialog->addExtensions(getNameFilters());
    fileDialog->setSelectedExtension(getSelectedExtension());
    fileDialog->setCurrentFile(get());
    fileDialog->setTitle(getDisplayName());
    fileDialog->setAcceptMode(getAcceptMode());
    fileDialog->setFileMode(getFileMode());
    fileDialog->setContentType(getContentType());

    if (fileDialog->show()) {
        set(fileDialog->getSelectedFile(), fileDialog->getSelectedFileExtension());
    }
}

const FileExtension& FileProperty::getSelectedExtension() const { return selectedExtension_; }
void FileProperty::setSelectedExtension(const FileExtension& ext) {
    if (selectedExtension_ != ext) {
        selectedExtension_ = ext;
        propertyModified();
    }
}

Document FileProperty::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = TemplateProperty<std::filesystem::path>::getDescription();
    auto table = doc.get({P("table", {{"class", "propertyInfo"}})});

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
