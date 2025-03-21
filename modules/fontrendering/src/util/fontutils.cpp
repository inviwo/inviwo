/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/fontrendering/util/fontutils.h>

#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/util/filesystem.h>                // for getDirectoryContents, getFileExte...
#include <inviwo/core/util/stdextensions.h>             // for contains, erase_remove_if
#include <inviwo/core/util/stringconversion.h>          // for toLower
#include <modules/fontrendering/fontrenderingmodule.h>  // for FontRenderingModule

#include <algorithm>  // for sort, transform
#include <cctype>     // for toupper
#include <iterator>   // for back_insert_iterator, back_inserter

#include <fmt/core.h>  // for format
#include <fmt/std.h>

namespace inviwo {

namespace font {

std::vector<std::pair<std::string, std::filesystem::path>> getAvailableFonts(
    const std::filesystem::path& fontPath) {

    const std::vector<std::string> supportedExt = {".ttf", ".otf", ".cff", ".pcf"};

    const std::filesystem::path path = (fontPath.empty() ? getDefaultFontPath() : fontPath);

    // scan for available fonts in the given path
    auto fonts = filesystem::getDirectoryContents(path, filesystem::ListMode::Files);

    // remove unsupported files
    std::erase_if(fonts, [supportedExt](const std::filesystem::path& font) {
        return !util::contains(supportedExt, font.extension().string());
    });

    // capitalize the first letter and each one following a space.
    // Also replace '-' with space for improved readability
    auto makeReadable = [](const std::filesystem::path& stem) {
        std::string dst(stem.string());
        auto it = dst.begin();
        *it = static_cast<char>(std::toupper(*it));
        while (it != dst.end()) {
            if ((*it == '-') || (*it == ' ')) {
                *it = ' ';
                if ((it + 1) != dst.end()) {
                    *(it + 1) = static_cast<char>(std::toupper(*(it + 1)));
                }
            }
            ++it;
        }
        return dst;
    };

    std::vector<std::pair<std::string, std::filesystem::path>> result;
    // create readable font names from file names and add full path to each file
    std::transform(fonts.begin(), fonts.end(), std::back_inserter(result),
                   [path, makeReadable](const std::filesystem::path& str)
                       -> std::pair<std::string, std::filesystem::path> {
                       return {makeReadable(str.stem()), path / str};
                   });

    // sort file names case insensitive
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return iCaseLess(a.first, b.first); });

    return result;
}

std::filesystem::path getDefaultFontPath() {
    return util::getModuleByType<FontRenderingModule>()->getPath() / "fonts";
}

std::filesystem::path getFont(FontType type, FullPath path) {
    auto [name, ext] = [type]() -> std::pair<std::filesystem::path, std::string> {
        switch (type) {
            case FontType::Default:
                return {"OpenSans-Semibold", ".ttf"};
            case FontType::Bold:
                return {"OpenSans-Bold", ".ttf"};
            case FontType::Caption:
                return {"OpenSans-Semibold", ".ttf"};
            case FontType::Label:
                return {"OpenSans-Regular", ".ttf"};
            default:
                return {"OpenSans-Semibold", ".ttf"};
        }
    }();

    if (path == FullPath::Yes) {
        name = getDefaultFontPath() / name;
        name += ext;
    }

    return name;
}

}  // namespace font

}  // namespace inviwo
