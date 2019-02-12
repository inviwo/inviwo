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

#pragma once

#include <inviwo/meta/inviwometadefine.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

#include <fmt/format.h>

namespace inviwo::meta::util {

template <typename T>
auto toUpper(const T& s) {
    std::string res;
    std::transform(s.begin(), s.end(), std::back_inserter(res),
                   [](char c) { return static_cast<char>(::toupper(c)); });
    return res;
}
template <typename T>
auto toLower(const T& s) {
    std::string res;
    std::transform(s.begin(), s.end(), std::back_inserter(res),
                   [](char c) { return static_cast<char>(::tolower(c)); });
    return res;
}

INVIWO_META_API std::string removePrefix(std::string_view prefix, std::string_view str);

INVIWO_META_API std::optional<std::filesystem::path> findShortestRelativePath(
    const std::filesystem::path& path, std::vector<std::filesystem::path> bases);

template <typename S, typename... Args>
std::runtime_error makeError(S&& s, Args&&... args) {
    return std::runtime_error{fmt::format(std::forward<S>(s), std::forward<Args>(args)...)};
}

}  // namespace inviwo::meta::util
