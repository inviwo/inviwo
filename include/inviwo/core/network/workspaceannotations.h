/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

class IVW_CORE_API WorkspaceAnnotations : public PropertyOwner {
public:
    struct IVW_CORE_API Base64Image : public Serializable {
        Base64Image() = default;
        Base64Image(std::string name, std::string base64jpeg, int w, int h);
        Base64Image(std::string name, std::string base64jpeg = "", ivec2 size = ivec2{0});
        virtual ~Base64Image() = default;

        bool isValid() const;

        virtual void serialize(Serializer &s) const override;
        virtual void deserialize(Deserializer &d) override;

        std::string name;
        std::string base64jpeg;
        ivec2 size = ivec2{0};
    };

    using ImageVector = std::vector<Base64Image>;

    WorkspaceAnnotations();
    WorkspaceAnnotations(const ImageVector &canvasImages);
    virtual ~WorkspaceAnnotations() = default;

    void setTitle(const std::string &title);
    std::string getTitle() const;

    void setAuthor(const std::string &author);
    std::string getAuthor() const;

    void setTags(const std::string &tags);
    std::string getTags() const;

    void setCategories(const std::string &cat);
    std::string getCategories() const;

    void setDescription(const std::string &desc);
    std::string getDescription() const;

    void setCanvasImages(const ImageVector &canvases);

    const ImageVector getCanvasImages() const;

    virtual void serialize(Serializer &s) const override;
    virtual void deserialize(Deserializer &d) override;

protected:
    StringProperty title_;
    StringProperty author_;
    StringProperty tags_;
    StringProperty categories_;
    StringProperty description_;

    ImageVector canvases_;
};

}  // namespace inviwo
