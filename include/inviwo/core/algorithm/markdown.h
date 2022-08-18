/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/document.h>

#include <string_view>

namespace inviwo {

namespace util {

/**
 * Parse a markdown string and convert to an inviwo document.
 * @see Document
 *
 * @param markdown A string that will be interpreted as markdown
 * @return an inviwo Document with an html representation of the parsed markdown
 */
IVW_CORE_API Document md2doc(std::string_view markdown);

/**
 * Parse a markdown string and convert to an inviwo document.
 * Before the string is parsed as markdown, any leading indentation is removed from the string
 * @see Document, indent::unindent
 *
 * @param markdown A string that will be interpreted as markdown
 * @return an inviwo Document with an html representation of the parsed markdown
 */
IVW_CORE_API Document unindentMd2doc(std::string_view markdown);

}  // namespace util

/**
 * Parse a markdown string and convert to an inviwo document.
 */
inline Document operator"" _md(const char* str, size_t len) {
    return util::md2doc(std::string_view(str, len));
}

/**
 * Parse a markdown string and convert to an inviwo document.
 */
inline Document operator"" _help(const char* str, size_t len) {
    return util::md2doc(std::string_view(str, len));
}

/**
 * Unindent and parse a markdown string and convert to an inviwo document.
 */
inline Document operator"" _unindentHelp(const char* str, size_t len) {
    return util::unindentMd2doc(std::string_view(str, len));
}

}  // namespace inviwo
