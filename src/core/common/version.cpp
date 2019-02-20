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

#include <inviwo/core/common/version.h>
#include <cstdio>

namespace inviwo {

Version::Version(std::string versionString) {
    std::sscanf(versionString.c_str(), "%u.%u.%u.%u", &major, &minor, &patch, &build);
}

Version::Version(const char* versionString) : Version(std::string(versionString)) {}

Version::Version(unsigned int major_, unsigned int minor_, unsigned int patch_, unsigned int build_)
    : major{major_}, minor{minor_}, patch{patch_}, build{build_} {}

bool Version::semanticVersionEqual(const Version& other) const {
    if (major < 1 || other.major < 1) {
        // Each version increment is a breaking change
        // when major version is 0
        return std::tie(major, minor, patch) == std::tie(other.major, other.minor, other.patch);
    } else {
        return std::tie(major, minor) == std::tie(other.major, other.minor);
    }
}

bool operator<(const Version& lhs, const Version& rhs) {
    // Keep ordering using lexicographical comparison provided by std::tie:
    return std::tie(lhs.major, lhs.minor, lhs.patch, lhs.build) <
           std::tie(rhs.major, rhs.minor, rhs.patch, rhs.build);
}
bool operator==(const Version& lhs, const Version& rhs) {
    // Keep ordering using lexicographical comparison provided by std::tie:
    return std::tie(lhs.major, lhs.minor, lhs.patch, lhs.build) ==
           std::tie(rhs.major, rhs.minor, rhs.patch, rhs.build);
}

bool operator!=(const Version& lhs, const Version& rhs) { return !(lhs == rhs); }

bool operator>(const Version& lhs, const Version& rhs) { return !(lhs <= rhs); }

bool operator>=(const Version& lhs, const Version& rhs) { return !(lhs < rhs); }

bool operator<=(const Version& lhs, const Version& rhs) { return (lhs < rhs) || (lhs == rhs); }

}  // namespace inviwo
