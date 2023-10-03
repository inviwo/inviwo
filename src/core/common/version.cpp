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

#include <inviwo/core/common/version.h>

#include <ostream>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

namespace {
constexpr Version ver{std::string_view{"9.8.3-pre+modules"}};
static_assert(ver.major == 9);
static_assert(ver.minor == 8);
static_assert(ver.patch == 3);
static_assert(ver.build() == "modules");
static_assert(ver.preRelease() == "pre");

static_assert(Version{"1.0.0"} < Version{"2.0.0"});
static_assert(Version{"2.0.0"} < Version{"2.1.0"});
static_assert(Version{"2.1.0"} < Version{"2.1.1"});
static_assert(Version{"1.0.0-alpha"} < Version{"1.0.0"});
static_assert(Version{"1.0.0-alpha"} < Version{"1.0.0-alpha.1"});
static_assert(Version{"1.0.0-alpha.1"} < Version{"1.0.0-alpha.beta"});
static_assert(Version{"1.0.0-alpha.beta"} < Version{"1.0.0-beta"});
static_assert(Version{"1.0.0-beta"} < Version{"1.0.0-beta.2"});
static_assert(Version{"1.0.0-beta.2"} < Version{"1.0.0-beta.11"});
static_assert(Version{"1.0.0-beta.11"} < Version{"1.0.0-rc.1"});
static_assert(Version{"1.0.0-rc.1"} < Version{"1.0.0"});

static_assert(Version{"2.0.0"} > Version{"1.0.0"});
static_assert(Version{"2.1.0"} > Version{"2.0.0"});
static_assert(Version{"2.1.1"} > Version{"2.1.0"});
static_assert(Version{"1.0.0"} > Version{"1.0.0-alpha"});
static_assert(Version{"1.0.0-alpha.1"} > Version{"1.0.0-alpha"});
static_assert(Version{"1.0.0-alpha.beta"} > Version{"1.0.0-alpha.1"});
static_assert(Version{"1.0.0-beta"} > Version{"1.0.0-alpha.beta"});
static_assert(Version{"1.0.0-beta.2"} > Version{"1.0.0-beta"});
static_assert(Version{"1.0.0-beta.11"} > Version{"1.0.0-beta.2"});
static_assert(Version{"1.0.0-rc.1"} > Version{"1.0.0-beta.11"});
static_assert(Version{"1.0.0"} > Version{"1.0.0-rc.1"});

static_assert(Version{"1.0.0+asdf"} < Version{"2.0.0+werg"});
static_assert(Version{"1.0.0-alpha.beta+bar"} > Version{"1.0.0-alpha.1+bar"});

static_assert(Version{"1.0.0"} == Version{"1.0.0"});
static_assert(Version{"1.0.0-beta"} == Version{"1.0.0-beta"});
static_assert(Version{"1.0.0+asdf"} == Version{"1.0.0+werg"});
static_assert(Version{"1.0.0-beta+asdf"} == Version{"1.0.0-beta+werg"});

}  // namespace

std::ostream& operator<<(std::ostream& ss, const Version& v) {
    fmt::print(ss, "{}", v);
    return ss;
}

}  // namespace inviwo
