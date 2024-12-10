/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <modules/fontrendering/fontrenderingmoduledefine.h>  // for IVW_MODULE_FONTRENDERING_API

#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/optionproperty.h>     // for OptionPropertyString
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, Property...
#include <modules/fontrendering/util/fontutils.h>      // for FontType, FontType::Default

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

/**
 * \ingroup properties
 * This option property lists all available font faces which can be used for font rendering.
 */
class IVW_MODULE_FONTRENDERING_API FontFaceOptionProperty
    : public OptionProperty<std::filesystem::path> {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.FontFaceOptionProperty"};

    FontFaceOptionProperty(std::string_view identifier, std::string_view displayName, Document help,
                           font::FontType fontType = font::FontType::Default,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);
    FontFaceOptionProperty(std::string_view identifier, std::string_view displayName,
                           font::FontType fontType = font::FontType::Default,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);
    FontFaceOptionProperty(std::string_view identifier, std::string_view displayName, Document help,
                           std::string_view fontFaceName,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);
    FontFaceOptionProperty(std::string_view identifier, std::string_view displayName,
                           std::string_view fontFaceName,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);
    FontFaceOptionProperty(const FontFaceOptionProperty& rhs);
    virtual FontFaceOptionProperty* clone() const override;
    virtual ~FontFaceOptionProperty() = default;
};

}  // namespace inviwo
