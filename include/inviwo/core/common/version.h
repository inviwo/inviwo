/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <inviwo/core/util/exception.h>

#include <string>
#include <iosfwd>
#include <tuple>
#include <array>

#include <fmt/format.h>

namespace inviwo {

/**
 * \class Version
 * \brief Parses a version string "Major.Minor.Patch.Build" and allow versions to be compared.
 * Try to follow semantic versioning: http://semver.org/
 * A nuanced picture, i.e. reasons why you do not necessarily need to follow semantic versioning:
 * "Why Semantic Versioning Isn't": https://gist.github.com/jashkenas/cbd2b088e20279ae2c8e
 *
 * 1. MAJOR version when you make incompatible API changes,
 * 2. MINOR version when you add functionality in a backwards-compatible manner, and
 * 3. PATCH version when you make backwards-compatible bug fixes.
 * 4. BUILD version can be used as metadata.
 *
 * Major and minor versions are used during equal comparison since
 * API changes should not exist in patch and build version changes
 * (unless major version is zero).
 *
 * @note Module versions are tied to the Inviwo core version, which means
 *       that there is no need to update module version if it is built for
 *       a new Inviwo core version.
 */
class IVW_CORE_API Version {
public:
    /**
     * \brief Parses the version.
     * @param version Dot separated version string "Major.Minor.Patch.Build"
     */
    constexpr Version(std::string_view version);
    constexpr Version(const char* version);
    constexpr Version(unsigned int major = 1, unsigned int minor = 0, unsigned int patch = 0,
                      unsigned int build = 0);

    /**
     * Major version >= 1: Return true if major and minor versions are equal, false otherwise.
     * Major version < 1: Return true if major, minor and patch versions are equal, false otherwise.
     * @note Major version zero (0.y.z) is for initial development. Anything may change at any time.
     * The public API should not be considered stable.
     * Patch and build versions are ignored since API should not have changed in those cases.
     * @return bool true if major and minor versions are equal, false otherwise.
     */
    constexpr bool semanticVersionEqual(const Version& other) const;

    /**
     * \brief Compares major, minor, patch and build versions in order.
     * @return bool true if lhs is less than rhs, false otherwise.
     */
    friend constexpr bool operator<(const Version& lhs, const Version& rhs) {
        // Keep ordering using lexicographical comparison provided by std::tie:
        return std::tie(lhs.major, lhs.minor, lhs.patch, lhs.build) <
               std::tie(rhs.major, rhs.minor, rhs.patch, rhs.build);
    }
    /**
     * \brief Compares major, minor, patch and build versions in order.
     * @return bool true if lhs is exactly the same as rhs, false otherwise.
     */
    friend constexpr bool operator==(const Version& lhs, const Version& rhs) {
        // Keep ordering using lexicographical comparison provided by std::tie:
        return std::tie(lhs.major, lhs.minor, lhs.patch, lhs.build) ==
               std::tie(rhs.major, rhs.minor, rhs.patch, rhs.build);
    }
    friend constexpr bool operator!=(const Version& lhs, const Version& rhs) {
        return !(lhs == rhs);
    }
    friend constexpr bool operator>(const Version& lhs, const Version& rhs) {
        return !(lhs <= rhs);
    }
    friend constexpr bool operator>=(const Version& lhs, const Version& rhs) {
        return !(lhs < rhs);
    }
    friend constexpr bool operator<=(const Version& lhs, const Version& rhs) {
        return (lhs < rhs) || (lhs == rhs);
    }

    IVW_CORE_API friend std::ostream& operator<<(std::ostream& ss, const Version& v);

    unsigned int major = 0;  ///< Increases when you make incompatible API changes
    unsigned int minor =
        0;  ///< Increases when you add functionality in a backwards-compatible manner
    unsigned int patch = 0;  ///< Increases when you make backwards-compatible bug fixes
    unsigned int build = 0;  ///< Version metadata
};

constexpr Version::Version(const char* version) : Version(std::string_view{version}) {}

constexpr Version::Version(std::string_view version) {
    const std::array<unsigned int*, 4> v{&major, &minor, &patch, &build};

    size_t begin = 0;
    size_t end = version.find('.', 0);
    for (auto e : v) {
        const auto num = version.substr(begin, end - begin);
        for (auto c : num) {
            if (c < '0' || c > '9') {
                throw Exception(IVW_CONTEXT, "Invalid character found: '{}' in version string '{}'",
                                c, version);
            }
            *e = *e * 10 + (c - '0');
        }
        if (end == std::string_view::npos) break;
        begin = end + 1;
        end = version.find('.', begin);
    }
}

constexpr Version::Version(unsigned int major, unsigned int minor, unsigned int patch,
                           unsigned int build)
    : major{major}, minor{minor}, patch{patch}, build{build} {}

constexpr bool Version::semanticVersionEqual(const Version& other) const {
    if (major < 1 || other.major < 1) {
        // Each version increment is a breaking change
        // when major version is 0
        return std::tie(major, minor, patch) == std::tie(other.major, other.minor, other.patch);
    } else {
        return std::tie(major, minor) == std::tie(other.major, other.minor);
    }
}

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::Version> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::Version& val, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        fmt::format_to(std::back_inserter(buff), "{}.{}.{}.{}", val.major, val.minor, val.patch,
                       val.build);
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};
