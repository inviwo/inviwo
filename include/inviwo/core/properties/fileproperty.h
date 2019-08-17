/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_FILEPROPERTY_H
#define IVW_FILEPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialogstate.h>

namespace inviwo {

class IVW_CORE_API FileRequestable {
public:
    virtual ~FileRequestable() = default;
    /**
     *	Ask the user to supply a file, return true if successful
     */
    virtual bool requestFile() = 0;
};

/**
 * \ingroup properties
 *  A class for file representations.
 *  Holds the value of the path to a file as a string.
 *
 * @see TemplateProperty
 */
class IVW_CORE_API FileProperty : public TemplateProperty<std::string> {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    /**
     * \brief Constructor for the FileProperty
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
    FileProperty(std::string identifier, std::string displayName, std::string value = "",
                 std::string contentType = "default",
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    FileProperty(const FileProperty& rhs) = default;

    FileProperty& operator=(const std::string& value);
    virtual FileProperty* clone() const override;
    virtual ~FileProperty() = default;

    /**
     * Set the file name and also update the selected extension to the first one matching file.
     */
    virtual void set(const std::string& file) override;
    /**
     * Set the file name and the selected extension.
     */
    virtual void set(const std::string& file, const FileExtension& selectedExtension);
    virtual void set(const Property* property) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void addNameFilter(std::string);
    virtual void addNameFilter(FileExtension);
    virtual void addNameFilters(const std::vector<FileExtension>& filters);
    virtual void clearNameFilters();
    virtual const std::vector<FileExtension>& getNameFilters() const;

    virtual void setAcceptMode(AcceptMode mode);
    AcceptMode getAcceptMode() const;

    virtual void setFileMode(FileMode mode);
    FileMode getFileMode() const;

    void setContentType(const std::string& contentType);
    const std::string& getContentType() const;

    const FileExtension& getSelectedExtension() const;
    void setSelectedExtension(const FileExtension& ext);

    /**
     *	Request a file from the user through the use of a widget or a FileDialog.
     */
    virtual void requestFile();

    virtual Document getDescription() const override;

private:
    std::vector<FileExtension> nameFilters_;
    FileExtension selectedExtension_;
    AcceptMode acceptMode_;
    FileMode fileMode_;
    std::string contentType_;
};

}  // namespace inviwo

#endif  // IVW_FILEPROPERTY_H
