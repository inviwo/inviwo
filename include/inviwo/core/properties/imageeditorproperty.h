/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/properties/fileproperty.h>

#include <vector>

namespace inviwo {

class IVW_CORE_API ImageLabel : public Serializable {
public:
    ImageLabel();
    ImageLabel(vec2 startPoint, vec2 rectSize, std::string name);
    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);
    std::string getName() const { return name_; }
    vec2 getTopLeft() const { return startPoint_; }
    vec2 getSize() const { return rectSize_; }

private:
    std::string name_;
    vec2 startPoint_;
    vec2 rectSize_;
};

/**
 * \ingroup properties
 * A class for file representations.
 * Holds the value of the path to a file as a string.
 *
 * @see FileProperty
 */

class IVW_CORE_API ImageEditorProperty : public FileProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.ImageEditorProperty"};

    ImageEditorProperty(std::string_view identifier, std::string_view displayName,
                        std::string_view value = "",
                        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                        PropertySemantics semantics = PropertySemantics::Default);

    ImageEditorProperty(const ImageEditorProperty& rhs);

    virtual ImageEditorProperty* clone() const override;
    virtual ~ImageEditorProperty();

    void addLabel(vec2 start, vec2 end, std::string name = "");
    void setDimensions(ivec2 imgSize);
    const std::vector<ImageLabel>& getLabels() const;
    void clearLabels();
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    std::vector<ImageLabel> labels_;
    ivec2 dimensions_;
};

}  // namespace inviwo
