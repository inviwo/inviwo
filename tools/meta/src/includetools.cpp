/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/meta/includetools.hpp>
#include <inviwo/meta/util.hpp>

#include <fstream>
#include <streambuf>
#include <regex>
#include <sstream>
#include <fmt/format.h>

namespace inviwo::meta::util {

bool replaceInclude(std::filesystem::path file, std::string_view oldInclude,
                    std::string_view newInclude) {

    const auto data = [&file]() {
        std::ifstream ifs{file};
        if (!ifs) throw util::makeError("Unable to open module file '{}'", file.generic_string());
        return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }();

    const auto reInclude = std::regex(fmt::format(R"(#include\s* {})", oldInclude));

    if (std::regex_search(data, reInclude)) {

        const auto replaced =
            std::regex_replace(data, reInclude, fmt::format(R"(#include {})", newInclude));

        std::ofstream ofs{file};
        if (!ofs) throw util::makeError("Unable to open module file '{}'", file.generic_string());
        ofs << replaced;

        return true;
    } else {
        return false;
    }
}

}  // namespace inviwo::meta::util
