/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <vector>

namespace inviwo {

namespace util {

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
 * \brief returns the default typeface of Inviwo along with the default font path and extension if
 * \p fullPath is true
 *
 * @param fullPath    the function returns the full path of the default font if true
 * @return default font name
 */
std::string IVW_MODULE_FONTRENDERING_API getDefaultFont(bool fullPath = false);

/**
 * \brief returns the default typeface with more weight
 *
 * @param fullPath    the function returns the full path of the bold default font if true
 * @return bold default font name
 * 
 * \see getDefaultFont
 */
std::string IVW_MODULE_FONTRENDERING_API getDefaultFontBold(bool fullPath = false);

/**
 * \brief returns the default typeface of Inviwo with the default font path
 *
 * @return full path to default font
 */
std::string IVW_MODULE_FONTRENDERING_API getDefaultFontCaptions();

/**
 * \brief returns the default typeface of Inviwo with the default font path
 *
 * @return full path to default font
 */
std::string IVW_MODULE_FONTRENDERING_API getDefaultFontLabels();

}  // namespace util

}  // namespace inviwo
