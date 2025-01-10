/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/properties/multifileproperty.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

std::string_view MultiFileProperty::getClassIdentifier() const { return classIdentifier; }

MultiFileProperty::MultiFileProperty(std::string_view identifier, std::string_view displayName,
                                     Document help, const std::vector<std::filesystem::path>& value,
                                     AcceptMode acceptMode, FileMode fileMode,
                                     std::string_view contentType,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , FileBase([this]() { propertyModified(); }, acceptMode, fileMode, contentType)
    , files_{"value"} {
    // Explicitly set the file name here. This ensures that the set file name is always serialized.
    set(value);
}

MultiFileProperty::MultiFileProperty(std::string_view identifier, std::string_view displayName,
                                     const std::vector<std::filesystem::path>& value,
                                     std::string_view contentType,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : MultiFileProperty(identifier, displayName, {}, value, AcceptMode::Open,
                        FileMode::ExistingFiles, contentType, invalidationLevel, semantics) {}

MultiFileProperty& MultiFileProperty::operator=(const std::vector<std::filesystem::path>& value) {
    set(value);
    return *this;
}

MultiFileProperty* MultiFileProperty::clone() const { return new MultiFileProperty(*this); }

void MultiFileProperty::set(const std::filesystem::path& value) {
    set(std::vector<std::filesystem::path>{value});
}

void MultiFileProperty::set(const std::vector<std::filesystem::path>& values) {
    bool modified = files_.update(values);

    // figure out best matching extension based on first filename
    if (auto* file = front()) {
        modified |= updateExtension(*file);
    } else {
        modified |= selectedExtension_.update(FileExtension{});
    }

    if (modified) propertyModified();
}

void MultiFileProperty::set(const std::vector<std::filesystem::path>& files,
                            const FileExtension& selectedExtension) {
    if (any_of(files_.update(files), selectedExtension_.update(selectedExtension))) {
        propertyModified();
    }
}

void MultiFileProperty::set(const MultiFileProperty* property) {
    if (!property) return;

    if (any_of(files_.update(property->files_), setFileBase(*property))) {
        propertyModified();
    }
}

void MultiFileProperty::set(const FileProperty* property) {
    if (!property) return;

    if (any_of(files_.update(std::vector{property->get()}), setFileBase(*property))) {
        propertyModified();
    }
}

void MultiFileProperty::set(const Property* property) {
    if (auto mfp = dynamic_cast<const MultiFileProperty*>(property)) {
        set(mfp);
    } else if (auto fp = dynamic_cast<const FileProperty*>(property)) {
        set(fp);
    }
}

bool MultiFileProperty::empty() const { return files_->empty(); }

const std::filesystem::path* MultiFileProperty::front() const {
    if (!files_->empty()) {
        return &files_->front();
    } else {
        return nullptr;
    }
}
const std::filesystem::path* MultiFileProperty::back() const {
    if (!files_->empty()) {
        return &files_->back();
    } else {
        return nullptr;
    }
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

    files_.serialize(s, serializationMode_);

    const auto& workspacePath = s.getFileDir();
    ;
    const auto& dataPath = filesystem::getPath(PathType::Data);

    // save absolute and relative paths for each file pattern
    std::vector<std::filesystem::path> workspaceRelativePaths;  // relative to workspace directory
    std::vector<std::filesystem::path> dataRelativePaths;       // relative to Inviwo data
    for (const auto& item : files_.value) {
        const auto basePath = item.parent_path();

        std::filesystem::path workspaceRelative;
        std::filesystem::path dataRelative;
        if (!basePath.empty()) {
            if (!workspacePath.empty() && workspacePath.root_name() == basePath.root_name()) {
                workspaceRelative = basePath.lexically_relative(workspacePath);
                if (workspaceRelative.empty()) {
                    workspaceRelative = ".";
                }
            }
            if (!dataPath.empty() && dataPath.root_name() == basePath.root_name()) {
                dataRelative = basePath.lexically_relative(dataPath);
                if (dataRelative.empty()) {
                    dataRelative = ".";
                }
            }
        }
        workspaceRelativePaths.push_back(workspaceRelative);
        dataRelativePaths.push_back(dataRelative);
    }

    s.serialize("workspaceRelativePath", workspaceRelativePaths, "files");
    s.serialize("ivwdataRelativePath", dataRelativePaths, "files");

    FileBase::serialize(s, serializationMode_);
}

void MultiFileProperty::deserialize(Deserializer& d) {
    namespace fs = std::filesystem;

    Property::deserialize(d);
    bool modified = files_.deserialize(d, serializationMode_);

    std::vector<fs::path> absolutePaths = files_.value;
    std::vector<fs::path> workspaceRelativePaths;  // relative to workspace directory
    std::vector<fs::path> dataRelativePaths;       // relative to Inviwo data

    d.deserialize("workspaceRelativePath", workspaceRelativePaths, "files");
    d.deserialize("ivwdataRelativePath", dataRelativePaths, "files");

    // check whether all path lists have the same number of entries
    if ((absolutePaths.size() == workspaceRelativePaths.size()) &&
        (absolutePaths.size() == dataRelativePaths.size())) {

        const auto& workspacePath = d.getFileDir();
        const auto& dataPath = filesystem::getPath(PathType::Data);

        for (std::size_t i = 0; i < absolutePaths.size(); ++i) {
            const auto basePath = absolutePaths[i].parent_path();
            const auto pattern = absolutePaths[i].filename();

            const auto workspaceBasedPath =
                (workspacePath / workspaceRelativePaths[i]).lexically_normal();
            const auto dataBasedPath = (dataPath / dataRelativePaths[i]).lexically_normal();

            if (!basePath.empty() && fs::is_directory(basePath)) {
                continue;
            } else if (!dataRelativePaths[i].empty() && fs::is_directory(dataBasedPath)) {
                absolutePaths[i] = dataBasedPath / pattern;
            } else if (!workspaceRelativePaths[i].empty() &&
                       std::filesystem::is_directory(workspaceBasedPath)) {
                absolutePaths[i] = workspaceBasedPath / pattern;
            }
        }
        modified |= files_.update(absolutePaths);
    }

    modified |= FileBase::deserialize(d, serializationMode_);

    if (modified) propertyModified();
}

void MultiFileProperty::requestFile() {
    for (auto widget : getWidgets()) {
        if (auto fileRequester = dynamic_cast<FileRequestable*>(widget)) {
            fileRequester->requestFile();
            return;
        }
    }
    // No FileRequestable widget found, use the factory.
    // Currently, the only difference between using the widget (Qt) and the FileDialog
    // directly is that the Qt widget remembers the previously used directory
    auto fileDialog =
        createFileDialog(getDisplayName(), front() ? *front() : std::filesystem::path{});
    if (fileDialog->show()) {
        set(fileDialog->getSelectedFiles(), fileDialog->getSelectedFileExtension());
    }
}

MultiFileProperty& MultiFileProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    FileBase::setAsDefault();
    files_.setAsDefault();
    return *this;
}
MultiFileProperty& MultiFileProperty::setDefault(const std::vector<std::filesystem::path>& value) {
    files_.defaultValue = value;
    return *this;
}
MultiFileProperty& MultiFileProperty::resetToDefaultState() {
    if (any_of(files_.reset(), FileBase::reset())) {
        propertyModified();
    }
    return *this;
}

bool MultiFileProperty::isDefaultState() const {
    // the multi file property does not have a default state.
    return false;
}

Document MultiFileProperty::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = Property::getDescription();
    auto table = doc.get({P("table", {{"class", "propertyInfo"}})});

    std::filesystem::path currentPath;
    // compile compact list of selected files, binning all files of the same directory

    StrBuffer buff;
    for (const auto& elem : files_.value) {
        auto dir = elem.parent_path();
        auto filename = elem.filename();
        if (dir != currentPath) {
            currentPath = dir;
            buff.append("<b>{}</b><br/>", dir.string());
        }
        buff.append("{}<br/>", filename.string());
    }

    utildoc::TableBuilder tb(table);
    switch (fileMode_) {
        case FileMode::AnyFile:
        case FileMode::ExistingFile:
        case FileMode::ExistingFiles: {
            tb(H("Files"), buff.view());
            break;
        }
        case FileMode::Directory: {
            tb(H("Directories"), buff.view());
            break;
        }
    }

    tb(H("Selected Extension"), selectedExtension_.value);
    tb(H("Accept Mode"), acceptMode_);
    tb(H("File Mode"), fileMode_);
    tb(H("Content Type"), contentType_);

    return doc;
}

}  // namespace inviwo
