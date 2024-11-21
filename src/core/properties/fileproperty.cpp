/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/properties/multifileproperty.h>

namespace inviwo {

FileBase::FileBase(std::function<void()> onModified, AcceptMode acceptMode, FileMode fileMode,
                   std::string_view contentType)
    : selectedExtension_{"selectedExtension"}
    , acceptMode_{"acceptMode", acceptMode}
    , fileMode_{"fileMode", fileMode}
    , contentType_{"contentType", contentType}
    , onModified_{onModified} {

    addNameFilter(FileExtension::all());
}

void FileBase::addNameFilter(std::string_view filter) {
    nameFilters_.push_back(FileExtension::createFileExtensionFromString(filter));
}

void FileBase::addNameFilter(FileExtension filter) { nameFilters_.push_back(filter); }

void FileBase::addNameFilters(const std::vector<FileExtension>& filters) {
    util::append(nameFilters_, filters);
}

void FileBase::clearNameFilters() { nameFilters_.clear(); }

const std::vector<FileExtension>& FileBase::getNameFilters() const { return nameFilters_; }

bool FileBase::matchesAnyNameFilter(const std::filesystem::path& file) const {
    for (const auto& filter : nameFilters_) {
        if (filter.matches(file)) {
            return true;
        }
    }
    return false;
}

void FileBase::setAcceptMode(AcceptMode acceptMode) {
    if (acceptMode_.update(acceptMode)) {
        onModified_();
    }
}

AcceptMode FileBase::getAcceptMode() const { return acceptMode_; }

void FileBase::setFileMode(FileMode fileMode) {
    if (fileMode_.update(fileMode)) {
        onModified_();
    }
}

FileMode FileBase::getFileMode() const { return fileMode_; }

void FileBase::setContentType(std::string_view contentType) {
    if (contentType_.update(contentType)) {
        onModified_();
    }
}

const std::string& FileBase::getContentType() const { return contentType_; }

bool FileBase::setFileBase(const FileBase& base) {
    bool modified = false;
    modified |= selectedExtension_.update(base.getSelectedExtension());
    modified |= acceptMode_.update(base.getAcceptMode());
    modified |= fileMode_.update(base.getFileMode());
    modified |= contentType_.update(base.getContentType());
    return modified;
}

const FileExtension& FileBase::getSelectedExtension() const { return selectedExtension_; }
void FileBase::setSelectedExtension(const FileExtension& ext) {
    if (selectedExtension_.update(ext)) {
        onModified_();
    }
}

bool FileBase::updateExtension(const std::filesystem::path& file) {
    bool modified = false;
    if (!selectedExtension_->matches(file)) {
        const auto it = std::find_if(nameFilters_.begin(), nameFilters_.end(), [&](const auto& f) {
            return !f.matchesAll() && f.matches(file);
        });
        if (it != nameFilters_.end() && selectedExtension_ != *it) {
            modified |= selectedExtension_.update(*it);
        } else {
            const auto allIt = std::find_if(nameFilters_.begin(), nameFilters_.end(),
                                            [&](const auto& f) { return f.matchesAll(); });
            if (allIt != nameFilters_.end() && selectedExtension_ != *allIt) {
                modified |= selectedExtension_.update(*allIt);
            }
        }
    }
    return modified;
}

void FileBase::setAsDefault() {
    selectedExtension_.setAsDefault();
    acceptMode_.setAsDefault();
    fileMode_.setAsDefault();
    contentType_.setAsDefault();
}
bool FileBase::reset() {
    auto modified = false;
    modified |= selectedExtension_.reset();
    modified |= acceptMode_.reset();
    modified |= fileMode_.reset();
    modified |= contentType_.reset();

    return modified;
}

std::unique_ptr<FileDialog> FileBase::createFileDialog(std::string_view title,
                                                       const std::filesystem::path& file) const {
    auto fileDialog = util::dynamic_unique_ptr_cast<FileDialog>(
        InviwoApplication::getPtr()->getDialogFactory()->create("FileDialog"));
    if (!fileDialog) {
        throw Exception(
            "Failed to create a FileDialog. Add one to the "
            "InviwoApplication::DialogFactory",
            IVW_CONTEXT);
    }
    fileDialog->setCurrentFile(file);
    fileDialog->addExtensions(getNameFilters());
    fileDialog->setSelectedExtension(getSelectedExtension());
    fileDialog->setTitle(title);
    fileDialog->setAcceptMode(getAcceptMode());
    fileDialog->setFileMode(getFileMode());
    fileDialog->setContentType(getContentType());

    return fileDialog;
}

void FileBase::serialize(Serializer& s, PropertySerializationMode mode) const {
    // We don't serialize the filename filter since these are typically depending on readers/writers
    // available at runtime
    selectedExtension_.serialize(s, mode);
    acceptMode_.serialize(s, mode);
    fileMode_.serialize(s, mode);
    contentType_.serialize(s, mode);
}
bool FileBase::deserialize(Deserializer& d, PropertySerializationMode mode) {
    bool modified = false;
    try {
        modified |= selectedExtension_.deserialize(d, mode);
        modified |= acceptMode_.deserialize(d, mode);
        modified |= fileMode_.deserialize(d, mode);
        modified |= contentType_.deserialize(d, mode);
    } catch (SerializationException& e) {
        LogInfo("Problem deserializing file Property: " << e.getMessage());
    }
    return modified;
}

std::string_view FileProperty::getClassIdentifier() const { return classIdentifier; }

FileProperty::FileProperty(std::string_view identifier, std::string_view displayName, Document help,
                           const std::filesystem::path& value, AcceptMode acceptMode,
                           FileMode fileMode, std::string_view contentType,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , FileBase([this]() { propertyModified(); }, acceptMode, fileMode, contentType)
    , file_{"value"} {
    // Explicitly set the file name here. This ensures that the set file name is always serialized.
    set(value);
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
    if (any_of(file_.update(file), updateExtension(file_.value))) {
        propertyModified();
    }
}

void FileProperty::set(const std::filesystem::path& file, const FileExtension& selectedExtension) {
    if (any_of(file_.update(file), selectedExtension_.update(selectedExtension))) {
        propertyModified();
    }
}

void FileProperty::set(const FileProperty* property) {
    if (!property) return;

    if (any_of(file_.update(property->file_), setFileBase(*property))) {
        propertyModified();
    }
}

void FileProperty::set(const MultiFileProperty* property) {
    if (!property) return;

    if (any_of(file_.update(property->front() ? *property->front() : std::filesystem::path{}),
               setFileBase(*property))) {
        propertyModified();
    }
}

void FileProperty::set(const Property* property) {
    if (auto fp = dynamic_cast<const FileProperty*>(property)) {
        set(fp);
    } else if (auto mfp = dynamic_cast<const MultiFileProperty*>(property)) {
        set(mfp);
    }
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

    const std::filesystem::path& absolutePath = get();
    std::filesystem::path workspaceRelativePath;
    std::filesystem::path dataRelativePath;

    if (!absolutePath.empty()) {
        auto workspacePath = s.getFileName().parent_path();
        if (!workspacePath.empty() && workspacePath.root_name() == absolutePath.root_name()) {
            workspaceRelativePath = std::filesystem::relative(absolutePath, workspacePath);
        }
        auto dataPath = filesystem::getPath(PathType::Data);
        if (!dataPath.empty() && dataPath.root_name() == absolutePath.root_name()) {
            dataRelativePath = std::filesystem::relative(absolutePath, dataPath);
        }
    }

    s.serialize("absolutePath", absolutePath);
    s.serialize("workspaceRelativePath", workspaceRelativePath);
    s.serialize("ivwdataRelativePath", dataRelativePath);

    FileBase::serialize(s, serializationMode_);
}

void FileProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    std::filesystem::path absolutePath;
    std::filesystem::path workspaceRelativePath;
    std::filesystem::path dataRelativePath;
    std::filesystem::path oldWorkspacePath;

    d.deserialize("absolutePath", absolutePath);
    d.deserialize("workspaceRelativePath", workspaceRelativePath);
    d.deserialize("ivwdataRelativePath", dataRelativePath);
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
    const auto dataPath = filesystem::getPath(PathType::Data);

    const auto workspaceBasedPath =
        std::filesystem::weakly_canonical(workspacePath / workspaceRelativePath);
    const auto dataBasedPath = std::filesystem::weakly_canonical(dataPath / dataRelativePath);

    bool modified = false;

    // Prefer the relative paths to make relocation easier.
    if (!dataRelativePath.empty() && std::filesystem::is_regular_file(dataBasedPath)) {
        modified = file_.update(dataBasedPath);
    } else if (!workspaceRelativePath.empty() &&
               std::filesystem::is_regular_file(workspaceBasedPath)) {
        modified |= file_.update(workspaceBasedPath);
    } else {
        modified |= file_.update(absolutePath);
    }

    modified |= FileBase::deserialize(d, serializationMode_);

    if (modified) propertyModified();
}

void FileProperty::requestFile() {
    for (auto widget : getWidgets()) {
        if (auto fileRequester = dynamic_cast<FileRequestable*>(widget)) {
            fileRequester->requestFile();
            return;
        }
    }
    // No FileRequestable widget found, use the factory.
    // Currently, the only difference between using the widget (Qt) and the FileDialog directly
    // is that the Qt widget stores the previously used directory
    auto fileDialog = createFileDialog(getDisplayName(), file_);
    if (fileDialog->show()) {
        set(fileDialog->getSelectedFile(), fileDialog->getSelectedFileExtension());
    }
}

FileProperty& FileProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    FileBase::setAsDefault();
    file_.setAsDefault();
    return *this;
}
FileProperty& FileProperty::setDefault(const std::filesystem::path& value) {
    file_.defaultValue = value;
    return *this;
}
FileProperty& FileProperty::resetToDefaultState() {
    if (any_of(file_.reset(), FileBase::reset())) {
        propertyModified();
    }
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

        case FileMode::Directory: {
            tb(H("Directory"), file_.value);
            break;
        }
    }

    tb(H("Selected Extension"), selectedExtension_.value);
    tb(H("Accept Mode"), acceptMode_.value);
    tb(H("File Mode"), fileMode_.value);
    tb(H("Content Type"), contentType_.value);

    return doc;
}

}  // namespace inviwo
