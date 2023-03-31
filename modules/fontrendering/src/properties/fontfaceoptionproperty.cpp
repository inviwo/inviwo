/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <modules/fontrendering/properties/fontfaceoptionproperty.h>

#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>     // for OptionPropertyString
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics
#include <inviwo/core/util/filesystem.h>               // for getFileNameWithoutExtension
#include <modules/fontrendering/util/fontutils.h>      // for getAvailableFonts, getFont, FontType

#include <utility>  // for pair
#include <vector>   // for vector

namespace inviwo {

const std::string FontFaceOptionProperty::classIdentifier = "org.inviwo.FontFaceOptionProperty";
std::string FontFaceOptionProperty::getClassIdentifier() const { return classIdentifier; }

FontFaceOptionProperty::FontFaceOptionProperty(std::string_view identifier,
                                               std::string_view displayName, Document help,
                                               font::FontType fontType,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : FontFaceOptionProperty(identifier, displayName, help, font::getFont(fontType),
                             invalidationLevel, semantics) {}

FontFaceOptionProperty::FontFaceOptionProperty(std::string_view identifier,
                                               std::string_view displayName,
                                               font::FontType fontType,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : FontFaceOptionProperty(identifier, displayName, {}, font::getFont(fontType),
                             invalidationLevel, semantics) {}

FontFaceOptionProperty::FontFaceOptionProperty(std::string_view identifier,
                                               std::string_view displayName, Document help,
                                               std::string_view fontFaceName,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : OptionPropertyString(identifier, displayName, invalidationLevel, semantics) {

    auto fonts = font::getAvailableFonts();

    for (auto font : fonts) {
        auto name = font.second.stem().string();
        // use the file name w/o extension as identifier
        addOption(name, font.first, font.second.string());
    }
    setSelectedIdentifier(fontFaceName);
    setCurrentStateAsDefault();
}

FontFaceOptionProperty::FontFaceOptionProperty(std::string_view identifier,
                                               std::string_view displayName,
                                               std::string_view fontFaceName,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : FontFaceOptionProperty(identifier, displayName, {}, fontFaceName, invalidationLevel,
                             semantics) {}

FontFaceOptionProperty::FontFaceOptionProperty(const FontFaceOptionProperty& rhs)
    : OptionPropertyString(rhs) {}

FontFaceOptionProperty* FontFaceOptionProperty::clone() const {
    return new FontFaceOptionProperty(*this);
}

}  // namespace inviwo
