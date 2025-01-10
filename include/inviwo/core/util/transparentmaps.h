/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <string>
#include <string_view>
#include <map>
#include <unordered_map>

namespace inviwo {

/**
 * \brief Transparent string hashing for use in unordered containers with string keys
 * for example: std::unordered_map<std::string, V, StringHash, std::equal_to<>>;
 */
struct IVW_CORE_API StringHash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
    std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
    std::size_t operator()(const std::pmr::string& str) const { return hash_type{}(str); }
};

// For unknown reasons std::string == std::pmr::string does not compile.
struct IVW_CORE_API StringComparePMR {
    using is_transparent = void;

    template <typename T1, typename T2>
    constexpr bool operator()(const T1& a, const T2& b) const
        requires std::constructible_from<std::string_view, const T1&> &&
                 std::constructible_from<std::string_view, const T2&>
    {
        return std::string_view{a} == std::string_view{b};
    }
};

struct IVW_CORE_API StringLessPMR {
    using is_transparent = void;

    template <typename T1, typename T2>
    constexpr bool operator()(const T1& a, const T2& b) const
        requires std::constructible_from<std::string_view, const T1&> &&
                 std::constructible_from<std::string_view, const T2&>
    {
        return std::string_view{a} < std::string_view{b};
    }
};

template <typename V>
using StringMap = std::map<std::string, V, StringLessPMR>;

template <typename V>
using StringMapPMR = std::pmr::map<std::pmr::string, V, StringLessPMR>;

template <typename V>
using UnorderedStringMap = std::unordered_map<std::string, V, StringHash, StringComparePMR>;

template <typename V>
using UnorderedStringMapPMR =
    std::pmr::unordered_map<std::pmr::string, V, StringHash, StringComparePMR>;

}  // namespace inviwo
