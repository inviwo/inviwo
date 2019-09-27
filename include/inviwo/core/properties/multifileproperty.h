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

#ifndef IVW_MULTIFILEPROPERTY_H
#define IVW_MULTIFILEPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialogstate.h>

namespace inviwo {

/**
 * \ingroup properties
 *  A class for file representations.
 *  Holds the value of the path to multiple files as strings.
 *
 * @see TemplateProperty, FileProperty
 */
class IVW_CORE_API MultiFileProperty : public TemplateProperty<std::vector<std::string>> {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    /**
     * \brief Constructor for the MultiFileProperty
     *
     * The PropertySemantics can be set to TextEditor. Then a TextEditorWidget will be used instead
     * of a FilePropertyWidget
     *
     * @param identifier identifier for the property
     * @param displayName displayName for the property
     * @param value the path to the file
     * @param contentType
     * @param invalidationLevel
     * @param semantics Can be set to Editor
     */
    MultiFileProperty(std::string identifier, std::string displayName,
                      const std::vector<std::string>& value = {},
                      std::string contentType = "default",
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);

    MultiFileProperty(const MultiFileProperty& rhs);
    MultiFileProperty& operator=(const std::vector<std::string>& value);
    virtual MultiFileProperty* clone() const override;
    virtual ~MultiFileProperty() = default;

    void set(const std::string& value);
    virtual void set(const std::vector<std::string>& values) override;
    virtual void set(const Property* property) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void addNameFilter(std::string);
    virtual void addNameFilter(FileExtension);
    virtual void addNameFilters(const std::vector<FileExtension>& filters);
    virtual void clearNameFilters();
    virtual std::vector<FileExtension> getNameFilters();

    virtual void setAcceptMode(AcceptMode mode);
    AcceptMode getAcceptMode() const;

    virtual void setFileMode(FileMode mode);
    FileMode getFileMode() const;

    void setContentType(const std::string& contentType);
    std::string getContentType() const;

    const FileExtension& getSelectedExtension() const;
    void setSelectedExtension(const FileExtension& ext);

    /**
     *	Request a file from the user through the use of a widget or a FileDialog.
     */
    void requestFile();

    virtual Document getDescription() const override;

private:
    std::vector<FileExtension> nameFilters_;
    FileExtension selectedExtension_;
    AcceptMode acceptMode_;
    FileMode fileMode_;
    std::string contentType_;
};

}  // namespace inviwo

#endif  // IVW_MULTIFILEPROPERTY_H
