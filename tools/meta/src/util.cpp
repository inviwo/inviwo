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

#include <inviwo/meta/util.hpp>

namespace inviwo::meta::util {

std::optional<std::filesystem::path> findShortestRelativePath(
    const std::filesystem::path& path, std::vector<std::filesystem::path> bases) {
    std::optional<std::filesystem::path> res;
    for (auto base : bases) {
        auto rel = std::filesystem::relative(path, base).lexically_normal();
        if (rel.empty()) continue;
        if (*rel.begin() == "..") continue;
        if (!res ||
            std::distance(rel.begin(), rel.end()) < std::distance(res->begin(), res->end())) {
            res = rel;
        }
    }
    return res;
}

std::string removePrefix(std::string_view prefix, std::string_view str) {
    if (str.size() >= prefix.size()) {
        if (str.substr(0, prefix.size()) == prefix) {
            return std::string{str.begin() + prefix.size(), str.end()};
        }
    }
    return std::string{str};
}

}  // namespace inviwo::meta::util
