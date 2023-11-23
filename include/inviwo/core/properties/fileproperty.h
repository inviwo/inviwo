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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialogstate.h>

#include <vector>

namespace inviwo {

class MultiFileProperty;
class FileDialog;

class IVW_CORE_API FileRequestable {
public:
    virtual ~FileRequestable() = default;
    /**
     *	Ask the user to supply a file, return true if successful
     */
    virtual bool requestFile() = 0;
};

class IVW_CORE_API FileBase {
public:
    static constexpr std::string_view defaultContentType = "default";

    FileBase(std::function<void()> onModified, AcceptMode acceptMode, FileMode fileMode,
             std::string_view contentType);

    void addNameFilter(std::string_view filter);
    void addNameFilter(FileExtension ext);
    void addNameFilters(const std::vector<FileExtension>& filters);
    void clearNameFilters();
    const std::vector<FileExtension>& getNameFilters() const;
    bool matchesAnyNameFilter(const std::filesystem::path& file) const;

    void setAcceptMode(AcceptMode mode);
    AcceptMode getAcceptMode() const;

    void setFileMode(FileMode mode);
    FileMode getFileMode() const;

    void setContentType(std::string_view contentType);
    const std::string& getContentType() const;

    const FileExtension& getSelectedExtension() const;
    void setSelectedExtension(const FileExtension& ext);

protected:
    bool setFileBase(const FileBase& base);
    bool updateExtension(const std::filesystem::path& file);

    void setAsDefault();
    bool reset();

    void serialize(Serializer& s, PropertySerializationMode mode) const;
    bool deserialize(Deserializer& d, PropertySerializationMode mode);

    std::unique_ptr<FileDialog> createFileDialog(std::string_view title,
                                                 const std::filesystem::path& file) const;

    /**
     * A variadic any_of. This will evaluate all the arguments.
     * But the order is arbitrary.
     */
    template <typename Arg, typename... Args>
    [[nodiscard]] static constexpr bool any_of(Arg&& arg, Args&&... args) {
        return (arg || ... || args);
    }

    std::vector<FileExtension> nameFilters_;
    ValueWrapper<FileExtension> selectedExtension_;
    ValueWrapper<AcceptMode> acceptMode_;
    ValueWrapper<FileMode> fileMode_;
    ValueWrapper<std::string> contentType_;

private:
    std::function<void()> onModified_;
};

/**
 * \ingroup properties
 *  A class for a file/directory path
 */
class IVW_CORE_API FileProperty : public Property, public FileBase {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;
    using value_type = std::filesystem::path;

    /**
     * \brief Constructor for the FileProperty
     *
     * The PropertySemantics can be set to TextEditor. Then a TextEditorWidget will be used
     * instead of a FilePropertyWidget
     *
     * @param identifier identifier for the property
     * @param displayName displayName for the property
     * @param help descriptive text
     * @param value the path to the file
     * @param acceptMode @see AcceptMode
     * @param fileMode @see FileMode
     * @param contentType
     * @param invalidationLevel
     * @param semantics Can be set to Editor
     */
    FileProperty(std::string_view identifier, std::string_view displayName, Document help,
                 const std::filesystem::path& value = {}, AcceptMode acceptMode = AcceptMode::Open,
                 FileMode fileMode = FileMode::AnyFile,
                 std::string_view contentType = defaultContentType,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    /**
     * \brief Constructor for the FileProperty
     *
     * The PropertySemantics can be set to TextEditor. Then a TextEditorWidget will be used
     * instead of a FilePropertyWidget
     *
     * @param identifier identifier for the property
     * @param displayName displayName for the property
     * @param help descriptive text
     * @param value the path to the file
     * @param contentType
     * @param invalidationLevel
     * @param semantics Can be set to Editor
     */
    FileProperty(std::string_view identifier, std::string_view displayName, Document help,
                 const std::filesystem::path& value, std::string_view contentType,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    /**
     * \brief Constructor for the FileProperty
     *
     * The PropertySemantics can be set to TextEditor. Then a TextEditorWidget will be used
     * instead of a FilePropertyWidget
     *
     * @param identifier identifier for the property
     * @param displayName displayName for the property
     * @param value the path to the file
     * @param contentType
     * @param invalidationLevel
     * @param semantics Can be set to Editor
     */
    FileProperty(std::string_view identifier, std::string_view displayName,
                 const std::filesystem::path& value = {},
                 std::string_view contentType = defaultContentType,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    FileProperty(const FileProperty& rhs) = default;

    FileProperty& operator=(const std::filesystem::path& value);
    virtual FileProperty* clone() const override;
    virtual ~FileProperty() = default;

    /**
     * Set the file name and also update the selected extension to the first one matching file.
     */
    void set(const std::filesystem::path& file);
    /**
     * Set the file name and the selected extension.
     */
    void set(const std::filesystem::path& file, const FileExtension& selectedExtension);

    void set(const FileProperty* property);
    /*
     * Assigns the first path of the MultiFileProperty to this.
     * If the MultiFileProperty is empty, this path will be empty.
     */
    void set(const MultiFileProperty* property);
    virtual void set(const Property* property) override;

    operator const std::filesystem::path&() const { return file_; }
    const std::filesystem::path& get() const { return file_; }
    const std::filesystem::path& operator*() const { return file_; };
    const std::filesystem::path* operator->() const { return &file_.value; }

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /**
     *	Request a file from the user through the use of a widget or a FileDialog.
     */
    virtual void requestFile();

    virtual Document getDescription() const override;

    virtual FileProperty& setCurrentStateAsDefault() override;
    FileProperty& setDefault(const std::filesystem::path& value);
    virtual FileProperty& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

    friend std::ostream& operator<<(std::ostream& os, const FileProperty& prop) {
        return os << prop.file_.value;
    }

private:
    ValueWrapper<std::filesystem::path> file_;
};

}  // namespace inviwo
