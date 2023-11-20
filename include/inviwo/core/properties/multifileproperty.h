/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialogstate.h>

#include <vector>

namespace inviwo {

/**
 * \ingroup properties
 *  A class for a list of file/directory paths
 * @see FileProperty
 */
class IVW_CORE_API MultiFileProperty : public Property, public FileBase {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;
    static constexpr std::string_view defaultContentType = "default";
    using value_type = std::vector<std::filesystem::path>;

    /**
     * \brief Constructor for the MultiFileProperty
     *
     * @param identifier identifier for the property
     * @param displayName displayName for the property
     * @param value the path to the file
     * @param contentType
     * @param invalidationLevel
     * @param semantics Can be set to Editor
     */
    MultiFileProperty(std::string_view identifier, std::string_view displayName,
                      const std::vector<std::filesystem::path>& value, std::string_view contentType,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);

    MultiFileProperty(std::string_view identifier, std::string_view displayName,
                      const std::vector<std::filesystem::path>& value = {},
                      AcceptMode acceptMode = AcceptMode::Open,
                      FileMode fileMode = FileMode::AnyFile,
                      std::string_view contentType = "default",
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);

    MultiFileProperty(const MultiFileProperty& rhs) = default;
    MultiFileProperty& operator=(const std::vector<std::filesystem::path>& value);
    virtual MultiFileProperty* clone() const override;
    virtual ~MultiFileProperty() = default;

    void set(const std::filesystem::path& value);
    void set(const std::vector<std::filesystem::path>& values);
    void set(const std::vector<std::filesystem::path>& files,
             const FileExtension& selectedExtension);
    void set(const MultiFileProperty* property);
    void set(const FileProperty* property);
    virtual void set(const Property* property) override;

    operator const std::vector<std::filesystem::path>&() const { return files_; }
    const std::vector<std::filesystem::path>& get() const { return files_; }
    const std::vector<std::filesystem::path>& operator*() const { return files_; };
    const std::vector<std::filesystem::path>* operator->() const { return &files_.value; }

    const std::filesystem::path* front() const;
    const std::filesystem::path* back() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /**
     *	Request a file from the user through the use of a widget or a FileDialog.
     */
    void requestFile();

    virtual Document getDescription() const override;

    virtual MultiFileProperty& setCurrentStateAsDefault() override;
    MultiFileProperty& setDefault(const std::vector<std::filesystem::path>& value);
    virtual MultiFileProperty& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

private:
    ValueWrapper<std::vector<std::filesystem::path>> files_;
};

}  // namespace inviwo
