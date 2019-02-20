/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/util/foreacharg.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <tuple>
#include <utility>

namespace inviwo {

namespace util {

namespace detail {

template <size_t N>
struct PrintEventHelper;

template <>
struct PrintEventHelper<1> {
    template <typename Arg>
    static void print(std::ostream& os, Arg&& item) {
        fmt::print(os, "{}", std::get<0>(item));
    }
};
template <>
struct PrintEventHelper<2> {
    template <typename Arg>
    static void print(std::ostream& os, Arg&& item) {
        fmt::print(os, "{}: {:8} ", std::get<0>(item), std::get<1>(item));
    }
};
template <>
struct PrintEventHelper<3> {
    template <typename Arg>
    static void print(std::ostream& os, Arg&& item) {
        fmt::print(os, "{}: {:{}} ", std::get<0>(item), std::get<1>(item), std::get<2>(item));
    }
};

}  // namespace detail

template <typename... Args>
void printEvent(std::ostream& os, const std::string& event, Args... args) {
    fmt::print(os, "{:14} ", event);
    util::for_each_argument(
        [&os](auto&& item) {
            detail::PrintEventHelper<std::tuple_size<
                typename std::remove_reference<decltype(item)>::type>::value>::print(os, item);
        },
        args...);
}

}  // namespace util

}  // namespace inviwo
