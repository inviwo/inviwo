/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/fontrendering/util/fontutils.h>

namespace inviwo {

PropertyClassIdentifier(FontProperty, "org.inviwo.FontProperty");

FontProperty::FontProperty(
    const std::string& identifier, const std::string& displayName,
    InvalidationLevel invalidationLevel,
    PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , fontFace_("fontFace", "Font Face")
    , fontSize_("fontSize", "Font size")
    , anchorPos_("anchor", "Anchor", vec2(1.0f, 0.0f), vec2(-1.0f), vec2(1.0f), vec2(0.01f))
{
    auto fonts = util::getAvailableFonts();

    for (auto font : fonts) {
        auto name = filesystem::getFileNameWithoutExtension(font.second);
        // use the file name w/o extension as identifier
        fontFace_.addOption(name, font.first, font.second);
    }
    fontFace_.setSelectedIdentifier("Montserrat-Medium");
    fontFace_.setCurrentStateAsDefault();

    // set up different font sizes
    std::vector<int> fontSizes = {6, 8, 10, 11, 12, 14, 16, 20, 24, 28, 36, 48, 60, 72, 96};
    for (auto size : fontSizes) {
        std::string str = std::to_string(size);
        fontSize_.addOption(str, str, size);
    }
    fontSize_.setSelectedIndex(5);
    fontSize_.setCurrentStateAsDefault();

    addProperty(fontFace_);
    addProperty(fontSize_);
    addProperty(anchorPos_);
}

FontProperty::FontProperty(const FontProperty& rhs)
    : CompositeProperty(rhs), fontFace_(rhs.fontFace_), fontSize_(rhs.fontSize_), anchorPos_(rhs.anchorPos_) {
    addProperty(fontFace_);
    addProperty(fontSize_);
    addProperty(anchorPos_);
}
/*
FontProperty& FontProperty::operator=(const FontProperty& rhs) {
    if (this != &rhs) {
        CompositeProperty::operator=(rhs);
        fontFace_ = rhs.fontFace_;
        fontSize_ = rhs.fontSize_;
        anchorPos_ = rhs.anchorPos_;
    }
    return *this;
}
*/

FontProperty* FontProperty::clone() const { return new FontProperty(*this); }

}  // namespace inviwo
