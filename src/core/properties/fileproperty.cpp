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

namespace {

constexpr auto update = [](auto& dst, const auto& src) {
    if (dst != src) {
        dst = src;
        return true;
    } else {
        return false;
    }
};

}

const std::string FileProperty::classIdentifier = "org.inviwo.FileProperty";
std::string FileProperty::getClassIdentifier() const { return classIdentifier; }

FileProperty::FileProperty(std::string_view identifier, std::string_view displayName, Document help,
                           const std::filesystem::path& value, AcceptMode acceptMode,
                           FileMode fileMode, std::string_view contentType,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , file_("value", {})
    , acceptMode_(acceptMode)
    , fileMode_(fileMode)
    , contentType_(contentType) {
    // Explicitly set the file name here since the TemplateProperty itself is initialized with an
    // empty string as default value. This ensures that the set file name is always serialized.
    set(value);
    addNameFilter(FileExtension::all());
}

FileProperty::FileProperty(std::string_view identifier, std::string_view displayName, Document help,
                           const std::filesystem::path& value, std::string_view contentType,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : FileProperty(identifier, displayName, std::move(help), value, AcceptMode::Open,
                   FileMode::AnyFile, contentType, invalidationLevel, semantics) {}

FileProperty::FileProperty(std::string_view identifier, std::string_view displayName,
                           const std::filesystem::path& value, std::string_view contentType,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : FileProperty(identifier, displayName, Document{}, value, AcceptMode::Open, FileMode::AnyFile,
                   contentType, invalidationLevel, semantics) {}

FileProperty& FileProperty::operator=(const std::filesystem::path& file) {
    set(file);
    return *this;
}

FileProperty* FileProperty::clone() const { return new FileProperty(*this); }

void FileProperty::set(const std::filesystem::path& file) {
    bool modified = file_.update(file);

    if (!selectedExtension_.matches(file_.value)) {
        const auto it = std::find_if(nameFilters_.begin(), nameFilters_.end(), [&](const auto& f) {
            return !f.matchesAll() && f.matches(file_.value);
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

void FileProperty::set(const std::filesystem::path& file, const FileExtension& selectedExtension) {
    bool modified = file_.update(file);
    modified |= update(selectedExtension_, selectedExtension);
    if (modified) propertyModified();
}

void FileProperty::set(const FileProperty* property) {
    if (!property) return;

    bool modified = file_.update(property->file_);

    modified |= update(selectedExtension_, property->selectedExtension_);
    modified |= update(acceptMode_, property->acceptMode_);
    modified |= update(fileMode_, property->fileMode_);
    modified |= update(contentType_, property->contentType_);

    if (modified) propertyModified();
}

void FileProperty::set(const Property* property) {
    set(dynamic_cast<const FileProperty*>(property));
}

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
    s.serialize("contentType", contentType_);
}

void FileProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    std::filesystem::path absolutePath;
    std::filesystem::path workspaceRelativePath;
    std::filesystem::path ivwDataRelativePath;
    std::filesystem::path oldWorkspacePath;

    d.deserialize("absolutePath", absolutePath);
    d.deserialize("workspaceRelativePath", workspaceRelativePath);
    d.deserialize("ivwdataRelativePath", ivwDataRelativePath);
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
    const auto ivwDataPath = filesystem::getPath(PathType::Data);

    const auto workspaceBasedPath =
        std::filesystem::weakly_canonical(workspacePath / workspaceRelativePath);
    const auto ivwdataBasedPath =
        std::filesystem::weakly_canonical(ivwDataPath / ivwDataRelativePath);

    bool modified = false;

    // Prefer the relative paths to make relocation easier.
    if (!ivwDataRelativePath.empty() && std::filesystem::is_regular_file(ivwdataBasedPath)) {
        modified = file_.update(ivwdataBasedPath);
    } else if (!workspaceRelativePath.empty() &&
               std::filesystem::is_regular_file(workspaceBasedPath)) {
        modified |= file_.update(workspaceBasedPath);
    } else {
        modified |= file_.update(absolutePath);
    }

    try {
        d.deserialize("selectedExtension", selectedExtension_);
        int acceptMode = static_cast<int>(acceptMode_);
        d.deserialize("acceptMode", acceptMode);
        modified |= update(acceptMode_, static_cast<AcceptMode>(acceptMode));

        int fileMode = static_cast<int>(fileMode_);
        d.deserialize("fileMode", fileMode);
        modified |= update(fileMode_, static_cast<FileMode>(fileMode));

        std::string contentType = contentType_;
        d.deserialize("contentType", contentType);
        modified |= update(contentType_, contentType);

    } catch (SerializationException& e) {
        LogInfo("Problem deserializing file Property: " << e.getMessage());
    }

    if (modified) propertyModified();
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

void FileProperty::setAcceptMode(AcceptMode acceptMode) {
    if (acceptMode_ != acceptMode) {
        acceptMode_ = acceptMode;
        propertyModified();
    }
}

AcceptMode FileProperty::getAcceptMode() const { return acceptMode_; }

void FileProperty::setFileMode(FileMode fileMode) {
    if (fileMode_ != fileMode) {
        fileMode_ = fileMode;
        propertyModified();
    }
}

FileMode FileProperty::getFileMode() const { return fileMode_; }

void FileProperty::setContentType(std::string_view contentType) {
    if (contentType_ != contentType) {
        contentType_ = contentType;
        propertyModified();
    }
}

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

FileProperty& FileProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    file_.setAsDefault();
    return *this;
}
FileProperty& FileProperty::setDefault(const std::filesystem::path& value) {
    file_.defaultValue = value;
    return *this;
}
FileProperty& FileProperty::resetToDefaultState() {
    if (file_.reset()) propertyModified();
    return *this;
}

bool FileProperty::isDefaultState() const {
    // the file property does not have a default state.
    return false;
}

Document FileProperty::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = Property::getDescription();
    auto table = doc.get({P("table", {{"class", "propertyInfo"}})});

    utildoc::TableBuilder tb(table);
    switch (fileMode_) {
        case FileMode::AnyFile:
        case FileMode::ExistingFile:
        case FileMode::ExistingFiles: {
            tb(H("File"), file_.value);
            break;
        }

        case FileMode::Directory:
        case FileMode::DirectoryOnly: {
            tb(H("Directory"), file_.value);
            break;
        }
    }

    tb(H("Accept Mode"), acceptMode_);
    tb(H("File Mode"), fileMode_);
    tb(H("Content Type"), contentType_);

    return doc;
}

}  // namespace inviwo
