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

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \struct TextBoundingBox
 *
 * \brief struct for holding bounding box information for a specific text
 *
 * The textual bounding box for a string has its origin (0,0) at the bottom-left corner and is
 * given by its extent. The bounding box enclosing all glyphs has its own origin relative to (0,0).
 * The first line starts at (0,0) + textExtent.y - ascender (i.e. getBaseLineOffset()).
 *
 * The glyph bounding box might be larger than the textual bounding box. It is guaranteed to
 * enclose all glyphs including overhang, e.g. caused by italic glyphs or glyphs exceeding
 * ascend and descend.
 */
struct IVW_MODULE_FONTRENDERING_API TextBoundingBox {

    TextBoundingBox() = default;

    TextBoundingBox(const size2_t &textExt, const ivec2 &glyphsOrigin, const size2_t &glyphsExt,
                    int baselineOffset);

    size2_t textExtent{0};  //<! extent of textual bounding box

    ivec2 glyphsOrigin{0};  //!< relative origin of bottom-left most glyph
    size2_t glyphsExtent{
        0};  //!< extent of bbox containing all glyphs extending to top right corner

    ivec2 glyphPenOffset{0};  //!< pen offset to align first glyph perfectly on first baseline

    void updateGlyphPenOffset(int baselineOffset);
};

}  // namespace inviwo
