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

#include <modules/fontrendering/properties/fontproperty.h>

#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                   // for IntProperty, FloatP...
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/util/glmvec.h>                                  // for vec2
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty
#include <modules/fontrendering/util/fontutils.h>                     // for getFont, FontType

namespace inviwo {

std::string_view FontProperty::getClassIdentifier() const { return classIdentifier; }

FontProperty::FontProperty(std::string_view identifier, std::string_view displayName,
                           std::string_view fontFaceName, int size, float lineSpacing,
                           vec2 anchorPos, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , fontFace_("fontFace", "Font Face", fontFaceName)
    , fontSize_("fontSize", "Font Size", size, 0, 144, 1)
    , lineSpacing_("lineSpacing", "Line Spacing", lineSpacing, -1.0f, 2.0f)
    , anchorPos_("anchor", "Anchor", anchorPos, vec2(-1.5f), vec2(1.5f), vec2(0.01f)) {

    fontSize_.setSemantics(PropertySemantics("Fontsize"));

    addProperty(fontFace_);
    addProperty(fontSize_);
    addProperty(lineSpacing_);
    addProperty(anchorPos_);
}

FontProperty::FontProperty(std::string_view identifier, std::string_view displayName,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : FontProperty(identifier, displayName, font::getFont(font::FontType::Default).string(), 14,
                   0.0f, vec2{-1.0f}, invalidationLevel, semantics) {}

FontProperty::FontProperty(std::string_view identifier, std::string_view displayName,
                           font::FontType fontType, int size, float lineSpacing, vec2 anchorPos,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : FontProperty(identifier, displayName, font::getFont(fontType).string(), size, lineSpacing,
                   anchorPos, invalidationLevel, semantics) {}

FontProperty::FontProperty(const FontProperty& rhs)
    : CompositeProperty(rhs)
    , fontFace_(rhs.fontFace_)
    , fontSize_(rhs.fontSize_)
    , lineSpacing_(rhs.lineSpacing_)
    , anchorPos_(rhs.anchorPos_) {
    addProperty(fontFace_);
    addProperty(fontSize_);
    addProperty(lineSpacing_);
    addProperty(anchorPos_);
}

FontProperty* FontProperty::clone() const { return new FontProperty(*this); }

const std::filesystem::path& FontProperty::getFontFace() const {
    return fontFace_.getSelectedValue();
}

int FontProperty::getFontSize() const { return fontSize_.get(); }

float FontProperty::getLineSpacing() const { return lineSpacing_.get(); }

vec2 FontProperty::getAnchorPos() const { return anchorPos_.get(); }

}  // namespace inviwo
