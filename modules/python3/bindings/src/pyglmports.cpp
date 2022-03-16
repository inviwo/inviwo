/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwopy/pyglmtypes.h>

#include <inviwo/core/util/glm.h>

#include <modules/python3/pyportutils.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <warn/pop>

#include <vector>

namespace py = pybind11;

namespace inviwo {

namespace {

template <typename T>
constexpr bool alwaysFalse() {
    return false;
}

struct ExposePortsFunctor {
    template <typename T>
    void operator()(pybind11::module& m) {
        using V = typename util::value_type<T>::type;
        constexpr auto N = util::extent<T>::value;

        if constexpr (N == 1) {
            constexpr auto name = []() {
                if constexpr (std::is_same_v<T, float>) {
                    return "float";
                } else if constexpr (std::is_same_v<T, double>) {
                    return "double";
                } else if constexpr (std::is_same_v<T, int>) {
                    return "int";
                } else if constexpr (std::is_same_v<T, unsigned int>) {
                    return "uint";
                } else {
                    static_assert(alwaysFalse<T>(), "Missing name for T");
                }
            }();
            const auto vecname = fmt::format("{}Vector", name);
            py::bind_vector<std::vector<T>>(m, vecname);
            exposeStandardDataPorts<std::vector<T>>(m, vecname);

        } else {
            const auto prefix = glm::detail::prefix<V>::value();
            const auto vecname = fmt::format("{}vec{}Vector", prefix, N);
            py::bind_vector<std::vector<T>>(m, vecname, py::buffer_protocol{});
            exposeStandardDataPorts<std::vector<T>>(m, vecname);
        }
    }
};

}  // namespace

void exposeGLMPorts(pybind11::module& m) {

    // clang-format off
    using types = std::tuple<
        float, vec2, vec3, vec4,
        double, dvec2, dvec3, dvec4,
        int, ivec2, ivec3, ivec4,
        unsigned int, uvec2, uvec3, uvec4
    >;
    // clang-format on
    util::for_each_type<types>{}(ExposePortsFunctor{}, m);
}

}  // namespace inviwo
