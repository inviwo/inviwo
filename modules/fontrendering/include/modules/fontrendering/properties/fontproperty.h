/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/fontrendering/fontrenderingmoduledefine.h>  // for IVW_MODULE_FONTREND...

#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                   // for FloatProperty, Floa...
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/util/glmvec.h>                                  // for vec2
#include <modules/fontrendering/datastructures/fontsettings.h>        // for FontSettings
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

/**
 * \ingroup properties
 * A composite property with the necessary parameters for font rendering like font face, font size,
 * and line spacing.
 */
class IVW_MODULE_FONTRENDERING_API FontProperty : public FontSettings, public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.FontProperty"};

    FontProperty(std::string_view identifier, std::string_view displayName,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    FontProperty(std::string_view identifier, std::string_view displayName,
                 std::string_view fontFaceName, int size = 14, float lineSpacing = 0.0f,
                 vec2 anchorPos = vec2{-1.0f},
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    FontProperty(std::string_view identifier, std::string_view displayName, font::FontType fontType,
                 int size = 14, float lineSpacing = 0.0f, vec2 anchorPos = vec2{-1.0f},
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    FontProperty(const FontProperty& rhs);
    virtual FontProperty* clone() const override;
    virtual ~FontProperty() = default;

    FontFaceOptionProperty fontFace_;
    IntProperty fontSize_;
    FloatProperty lineSpacing_;
    FloatVec2Property anchorPos_;

    // Inherited via FontSettings
    virtual const std::filesystem::path& getFontFace() const override;
    virtual int getFontSize() const override;
    virtual float getLineSpacing() const override;
    virtual vec2 getAnchorPos() const override;
};

}  // namespace inviwo
