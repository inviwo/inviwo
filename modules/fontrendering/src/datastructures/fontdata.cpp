/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/fontrendering/datastructures/fontdata.h>

#include <inviwo/core/util/glmvec.h>                            // for vec2
#include <modules/fontrendering/datastructures/fontsettings.h>  // for FontSettings

#include <type_traits>  // for is_copy_assignable_v, is_...

namespace inviwo {
FontData::FontData(const FontSettings& s)
    : fontFace{s.getFontFace()}
    , fontSize{s.getFontSize()}
    , lineSpacing{s.getLineSpacing()}
    , anchorPos{s.getAnchorPos()} {}
const std::filesystem::path& FontData::getFontFace() const { return fontFace; }
int FontData::getFontSize() const { return fontSize; }
float FontData::getLineSpacing() const { return lineSpacing; }
vec2 FontData::getAnchorPos() const { return anchorPos; }

static_assert(std::is_copy_constructible_v<FontData>);
static_assert(std::is_copy_assignable_v<FontData>);
static_assert(std::is_nothrow_move_constructible_v<FontData>);
static_assert(std::is_nothrow_move_assignable_v<FontData>);

}  // namespace inviwo
