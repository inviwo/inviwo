/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

namespace inviwo {

namespace font {

enum class FullPath { Yes, No };
enum class FontType { Default, Bold, Caption, Label };

/**
 * \brief returns a list of all fonts found in the given directory,  font directory of the
 * fontrendering module
 *
 * @param  fontPath   path to fonts. If empty, the directory returned by getDefaultPath() will be
 * used.
 *
 * @return font list containing pairs for all available fonts. Each pair holds the font name
 *     and the full path. Fonts are sorted alphabetically
 */
std::vector<std::pair<std::string, std::string>> IVW_MODULE_FONTRENDERING_API
getAvailableFonts(const std::string& fontPath = std::string());

/**
 * \brief returns the default font directory of Inviwo
 *
 * @return default path containing fonts shipped with Inviwo. Corresponds to the font directory
 *                 located in the font rendering module directory.
 */
std::string IVW_MODULE_FONTRENDERING_API getDefaultFontPath();

/**
 * \brief returns the default typeface of Inviwo for \p type along with the default font path and
 * extension if \p fullPath is true
 *
 * @param type    requested font type
 * @param path    if equal to FullPath::Yes, then the full path of the font including extension is
 *                returned, only font name otherwise
 * @return font name
 */
std::string IVW_MODULE_FONTRENDERING_API getFont(FontType type, FullPath path = FullPath::No);

}  // namespace font

// namespace util for backward compatibility
namespace util {

using font::getAvailableFonts;
using font::getDefaultFontPath;

}  // namespace util

}  // namespace inviwo
