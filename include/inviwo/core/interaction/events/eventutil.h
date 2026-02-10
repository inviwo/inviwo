/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/glmfmt.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <tuple>
#include <utility>

namespace inviwo {

namespace util {

template <typename... Args>
void printEvent(fmt::memory_buffer& buff, std::string_view event, const Args&... args) {
    fmt::format_to(std::back_inserter(buff), "{:14} ", event);
    (
        [&]<typename T>(const T& arg) {
            if constexpr (std::tuple_size_v<T> == 3) {
                fmt::format_to(std::back_inserter(buff), "{}: {:{}} ", std::get<0>(arg),
                               std::get<1>(arg), std::get<2>(arg));
            } else if constexpr (std::tuple_size_v<T> == 2) {
                fmt::format_to(std::back_inserter(buff), "{}: {:} ", std::get<0>(arg),
                               std::get<1>(arg));
            } else if constexpr (std::tuple_size_v<T> == 1) {
                fmt::format_to(std::back_inserter(buff), "{}", std::get<0>(arg));
            }
        }(args),
        ...);
}

}  // namespace util

}  // namespace inviwo
