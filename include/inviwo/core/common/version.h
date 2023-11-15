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
#include <optional>

#include <fmt/format.h>

namespace inviwo {

/**
 * \class Version
 * \brief Parses a version string "Major.Minor.Patch-PreRelease+Build" and allow versions to be
 * compared. Try to follow semantic versioning: http://semver.org/ A nuanced picture, i.e. reasons
 * why you do not necessarily need to follow semantic versioning: "Why Semantic Versioning Isn't":
 * https://gist.github.com/jashkenas/cbd2b088e20279ae2c8e
 *
 * 1. MAJOR version when you make incompatible API changes,
 * 2. MINOR version when you add functionality in a backwards-compatible manner, and
 * 3. PATCH version when you make backwards-compatible bug fixes.
 * 4. PRE-RELEASE Annotate pre-release versions with a hyphen and a series of dot separated
 *    identifiers.
 * 5. BUILD Build metadata denoted by appending a plus sign and a series of dot separated
 *    identifiers immediately following the patch or pre-release version. The build metadata is
 *    ignored when comparing versions.
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
                      std::string_view preRelease = "", std::string_view build = "");

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
     * \brief Compares major, minor, patch and prerelease versions in order.
     */
    constexpr std::strong_ordering operator<=>(const Version& rhs) const {
        // Keep ordering using lexicographical comparison provided by std::tie:
        const auto order =
            std::tie(major, minor, patch) <=> std::tie(rhs.major, rhs.minor, rhs.patch);

        if (order != std::strong_ordering::equal) return order;

        // Compare pre-release versions
        return Version::comparePreRelease(preRelease(), rhs.preRelease());
    }
    constexpr bool operator==(const Version& rhs) const {
        return operator<=>(rhs) == std::strong_ordering::equal;
    }

    IVW_CORE_API friend std::ostream& operator<<(std::ostream& ss, const Version& v);

    unsigned int major = 0;  ///< Increases when you make incompatible API changes
    unsigned int minor =
        0;  ///< Increases when you add functionality in a backwards-compatible manner
    unsigned int patch = 0;  ///< Increases when you make backwards-compatible bug fixes

    constexpr std::string_view preRelease() const { return preReleaseBuffer.data(); }
    constexpr std::string_view build() const { return buildBuffer.data(); }

private:
    static constexpr std::strong_ordering comparePreRelease(std::string_view lhsPre,
                                                            std::string_view rhsPre) {
        if (lhsPre.empty() && rhsPre.empty()) {
            return std::strong_ordering::equal;
        } else if (lhsPre.empty()) {
            return std::strong_ordering::greater;
        } else if (rhsPre.empty()) {
            return std::strong_ordering::less;
        } else {
            size_t lhsBegin = 0;
            size_t rhsBegin = 0;

            while (true) {
                size_t lhsEnd = lhsPre.find_first_of(".", lhsBegin);
                size_t rhsEnd = rhsPre.find_first_of(".", rhsBegin);

                auto lhsPart = lhsPre.substr(lhsBegin, lhsEnd - lhsBegin);
                auto rhsPart = rhsPre.substr(rhsBegin, rhsEnd - rhsBegin);

                auto lhsNum = toNumber(lhsPart);
                auto rhsNum = toNumber(rhsPart);

                if (lhsNum && rhsNum) {
                    const auto numOrder = *lhsNum <=> *rhsNum;
                    if (numOrder != std::strong_ordering::equal) {
                        return numOrder;
                    }
                } else if (lhsNum) {
                    return std::strong_ordering::less;
                } else if (rhsNum) {
                    return std::strong_ordering::greater;
                } else {
                    const auto strOrder = lhsPart <=> rhsPart;
                    if (strOrder != std::strong_ordering::equal) {
                        return strOrder;
                    }
                }

                lhsBegin = lhsEnd != std::string_view::npos ? lhsEnd + 1 : lhsEnd;
                rhsBegin = rhsEnd != std::string_view::npos ? rhsEnd + 1 : rhsEnd;

                if (lhsBegin >= lhsPre.size() && rhsBegin >= rhsPre.size()) {
                    return std::strong_ordering::equal;
                } else if (lhsBegin >= lhsPre.size()) {
                    return std::strong_ordering::less;
                } else if (rhsBegin >= rhsPre.size()) {
                    return std::strong_ordering::greater;
                }
            }
        }
    }
    static constexpr std::optional<unsigned int> toNumber(std::string_view num) {
        unsigned int result = 0;
        for (auto c : num) {
            if (c < '0' || c > '9') {
                return std::nullopt;
            }
            result = result * 10 + (c - '0');
        }
        return result;
    }

    std::array<char, 32> preReleaseBuffer{0};  ///< Pre-release version annotation
    std::array<char, 32> buildBuffer{0};       ///< Build metadata
};

constexpr Version::Version(const char* version) : Version(std::string_view{version}) {}

constexpr Version::Version(std::string_view version) {
    const std::array<unsigned int*, 3> v{&major, &minor, &patch};

    auto majorMinorPatch = version.substr(0, version.find_first_of("-+"));

    size_t begin = 0;
    size_t end = majorMinorPatch.find('.', 0);
    for (auto e : v) {
        const auto num = majorMinorPatch.substr(begin, end - begin);
        if (auto val = toNumber(num)) {
            *e = *val;
        } else {
            throw Exception(IVW_CONTEXT, "Invalid number found: '{}' in version string '{}'", num,
                            version);
        }
        if (end == std::string_view::npos) break;
        begin = end + 1;
        end = majorMinorPatch.find('.', begin);
    }

    const auto preReleaseBegin = version.find_first_of("-", 0);
    if (preReleaseBegin != std::string_view::npos) {
        auto preReleaseEnd = version.find_first_of("+", preReleaseBegin);
        if (preReleaseEnd == std::string_view::npos) {
            preReleaseEnd = version.size();
        }
        const auto preRelease =
            version.substr(preReleaseBegin + 1, preReleaseEnd - preReleaseBegin - 1);
        if (preRelease.size() > preReleaseBuffer.size() - 1) {
            throw Exception(IVW_CONTEXT, "Pre-release version string too long: len('{}') >= {} ",
                            preRelease, preReleaseBuffer.size());
        }
        std::copy(preRelease.begin(), preRelease.end(), preReleaseBuffer.begin());
    }

    auto buildBegin = version.find_first_of("+", 0);
    if (buildBegin != std::string_view::npos) {
        const auto build = version.substr(buildBegin + 1);
        if (build.size() > buildBuffer.size() - 1) {
            throw Exception(IVW_CONTEXT, "Build version string too long: len('{}') >= {} ", build,
                            buildBuffer.size());
        }
        std::copy(build.begin(), build.end(), buildBuffer.begin());
    }
}

constexpr Version::Version(unsigned int major, unsigned int minor, unsigned int patch,
                           std::string_view preRelease, std::string_view build)
    : major{major}, minor{minor}, patch{patch} {

    if (preRelease.size() > preReleaseBuffer.size() - 1) {
        throw Exception(IVW_CONTEXT, "Pre-release version string too long: len('{}') >= {} ",
                        preRelease, preReleaseBuffer.size());
    }
    if (build.size() > buildBuffer.size() - 1) {
        throw Exception(IVW_CONTEXT, "Build version string too long: len('{}') >= {} ", build,
                        buildBuffer.size());
    }
    std::copy(preRelease.begin(), preRelease.end(), preReleaseBuffer.begin());
    std::copy(build.begin(), build.end(), buildBuffer.begin());
}

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

        fmt::format_to(std::back_inserter(buff), "{}.{}.{}", val.major, val.minor, val.patch);

        if (!val.preRelease().empty()) {
            fmt::format_to(std::back_inserter(buff), "-{}", val.preRelease());
        }
        if (!val.build().empty()) {
            fmt::format_to(std::back_inserter(buff), "+{}", val.build());
        }
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};
