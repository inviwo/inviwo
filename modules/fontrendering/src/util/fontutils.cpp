/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <modules/fontrendering/fontrenderingmodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>

#include <algorithm>

namespace inviwo {

namespace util {

std::vector<std::pair<std::string, std::string>> getAvailableFonts(const std::string &fontPath) {
    const std::vector<std::string> supportedExt = {"ttf", "otf", "cff", "pcf"};

    const std::string path = (fontPath.empty() ? getDefaultFontPath() : fontPath);

    // scan for available fonts in the given path
    auto fonts = filesystem::getDirectoryContents(path, filesystem::ListMode::Files);

    // remove unsupported files
    util::erase_remove_if(fonts, [supportedExt](const std::string &str) {
        return !util::contains(supportedExt, filesystem::getFileExtension(str));
    });

    // sort file names case insensitive
    std::sort(fonts.begin(), fonts.end(),
              [](std::string a, std::string b) { return toLower(a) < toLower(b); });

    // capitalize the first letter and each one following a space.
    // Also replace '-' with space for improved readability
    auto makeReadable = [](const std::string &str) {
        std::string dst(str);
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

    std::vector<std::pair<std::string, std::string>> result;
    // create readable font names from file names and add full path to each file
    std::transform(
        fonts.begin(), fonts.end(), std::back_inserter(result),
        [path, makeReadable](const std::string &str) -> std::pair<std::string, std::string> {
            return {makeReadable(filesystem::getFileNameWithoutExtension(str)), path + '/' + str};
        });

    return result;
}

std::string getDefaultFontPath() {
    return InviwoApplication::getPtr()->getModuleByType<FontRenderingModule>()->getPath() +
           "/fonts";
}

}  // namespace util

}  // namespace inviwo
