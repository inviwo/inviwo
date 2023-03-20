/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>

#include <inviwo/core/util/staticstring.h>

#include <glm/gtc/type_precision.hpp>
#include <glm/fwd.hpp>

#include <string_view>
#include <filesystem>
#include <string>

namespace inviwo {

template <typename T, size_t N>
struct InviwoDefaultData {
    constexpr InviwoDefaultData(StaticString<N> aname, uvec2 adim, T aval, T amin, T amax, T ainc)
        : name{aname}, dim{adim}, val{aval}, min{amin}, max{amax}, inc{ainc} {}

    StaticString<N> name;
    uvec2 dim;
    T val;
    T min;
    T max;
    T inc;
};

template <typename T, size_t N>
InviwoDefaultData(StaticString<N>, uvec2, T, T, T, T) -> InviwoDefaultData<T, N>;

template <typename T>
struct InviwoDefaults {};

template <>
struct InviwoDefaults<char> {
    static constexpr auto get() {
        return InviwoDefaultData{StaticString{"Char"}, uvec2(1, 1), char{0},
                                 char{-100},           char{100},   char{1}};
    }
};

template <>
struct InviwoDefaults<unsigned char> {
    static constexpr auto get() {
        using uchar = unsigned char;
        return InviwoDefaultData{
            StaticString{"UChar"}, uvec2(1, 1), uchar{0}, uchar{0}, uchar{100}, uchar{1}};
    }
};
template <>
struct InviwoDefaults<short> {
    static constexpr auto get() {
        return InviwoDefaultData{StaticString{"Short"}, uvec2(1, 1), short{0},
                                 short{-100},           short{100},  short{1}};
    }
};
template <>
struct InviwoDefaults<unsigned short> {
    static constexpr auto get() {
        using ushort = unsigned short;
        return InviwoDefaultData{
            StaticString{"UShort"}, uvec2(1, 1), ushort{0}, ushort{0}, ushort{100}, ushort{1}};
    }
};
template <>
struct InviwoDefaults<int> {
    static constexpr auto get() {
        return InviwoDefaultData{StaticString{"Int"}, uvec2(1, 1), int{0},
                                 int{-100},           int{100},    int{1}};
    }
};
template <>
struct InviwoDefaults<unsigned int> {
    static constexpr auto get() {
        using uint = unsigned int;
        return InviwoDefaultData{
            StaticString{"UInt"}, uvec2(1, 1), uint{0}, uint{0}, uint{100}, uint{1}};
    }
};
template <>
struct InviwoDefaults<glm::i64> {
    static constexpr auto get() {
        return InviwoDefaultData{StaticString{"Int64"}, uvec2(1, 1),    glm::i64{0},
                                 glm::i64{0},           glm::i64{1024}, glm::i64{1}};
    }
};
template <>
struct InviwoDefaults<size_t> {
    static constexpr auto get() {
        return InviwoDefaultData{
            StaticString{"Size_t"}, uvec2(1, 1), size_t{0}, size_t{0}, size_t{100}, size_t{1}};
    }
};
template <>
struct InviwoDefaults<float> {
    static constexpr auto get() {
        return InviwoDefaultData{StaticString{"Float"}, uvec2(1, 1), float{0.0f},
                                 float{0.0f},           float{1.0f}, float{0.01f}};
    }
};
template <>
struct InviwoDefaults<double> {
    static constexpr auto get() {
        return InviwoDefaultData{StaticString{"Double"},
                                 uvec2(1, 1),
                                 double{0.0},
                                 double{0.0},
                                 double{1.0},
                                 double{0.01}};
    }
};

template <>
struct InviwoDefaults<std::string> {
    static auto get() {
        return InviwoDefaultData{StaticString{"String"}, uvec2(1, 1),   std::string{},
                                 std::string{},          std::string{}, std::string{}};
    }
};

template <>
struct InviwoDefaults<std::filesystem::path> {
    static auto get() {
        return InviwoDefaultData{StaticString{"Path"},    uvec2(1, 1),
                                 std::filesystem::path{}, std::filesystem::path{},
                                 std::filesystem::path{}, std::filesystem::path{}};
    }
};

template <>
struct InviwoDefaults<bool> {
    static auto get() {
        return InviwoDefaultData{StaticString{"Bool"}, uvec2(1, 1), false, false, true, true};
    }
};

template <>
struct InviwoDefaults<glm::dquat> {
    static auto get() {
        return InviwoDefaultData{
            StaticString{"DoubleQuaternion"}, uvec2(4, 1),
            glm::dquat(0., 0., 0., 0.),       glm::dquat(-1., -1., -1., -1.),
            glm::dquat(1., 1., 1., 1.),       glm::dquat(0.01, 0.01, 0.01, 0.01)};
    }
};
template <>
struct InviwoDefaults<glm::fquat> {
    static auto get() {
        return InviwoDefaultData{
            StaticString{"FloatQuaternion"}, uvec2(4, 1),
            glm::fquat(0.f, 0.f, 0.f, 0.f),  glm::fquat(-1.f, -1.f, -1.f, -1.f),
            glm::fquat(1.f, 1.f, 1.f, 1.f),  glm::fquat(0.01f, 0.01f, 0.01f, 0.01f)};
    }
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct InviwoDefaults<glm::vec<L, T, Q>> {
    using type = glm::vec<L, T, Q>;
    static constexpr auto get() {
        constexpr std::array<StaticString<1>, 4> num = {"1", "2", "3", "4"};
        constexpr auto t = InviwoDefaults<T>::get();
        return InviwoDefaultData{t.name + "Vec" + num[L - 1],
                                 uvec2(L, 1),
                                 type{t.val},
                                 type{t.min},
                                 type{t.max},
                                 type{t.inc}};
    }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct InviwoDefaults<glm::mat<C, R, T, Q>> {
    using type = glm::mat<C, R, T, Q>;
    static constexpr auto get() {
        constexpr std::array<StaticString<1>, 4> num = {"1", "2", "3", "4"};
        constexpr auto t = InviwoDefaults<T>::get();
        if constexpr (C == R) {
            return InviwoDefaultData{t.name + "Mat" + num[C - 1],
                                     uvec2(C, R),
                                     type{0} + t.val,
                                     type{0} + t.min,
                                     type{0} + t.max,
                                     type{0} + t.inc};
        } else {
            return InviwoDefaultData{t.name + "Mat" + num[C - 1] + "x" + num[R - 1],
                                     uvec2(C, R),
                                     type{0} + t.val,
                                     type{0} + t.min,
                                     type{0} + t.max,
                                     type{0} + t.inc};
        }
    }
};

template <glm::length_t L, glm::qualifier Q>
struct InviwoDefaults<glm::vec<L, size_t, Q>> {
    using type = glm::vec<L, size_t, Q>;
    static constexpr auto get() {
        constexpr auto t = InviwoDefaults<size_t>::get();
        constexpr std::array<StaticString<1>, 4> num = {"1", "2", "3", "4"};
        return InviwoDefaultData{"IntSize" + num[L - 1],
                                 uvec2(L, 1),
                                 type{t.val},
                                 type{t.min},
                                 type{t.max},
                                 type{t.inc}};
    }
};

template <typename T>
struct Defaultvalues {
public:
    static constexpr auto data() { return InviwoDefaults<T>::get(); };
    static auto getName() { return data().name; }
    static uvec2 getDim() { return data().dim; }
    static T getVal() { return data().val; }
    static T getMin() { return data().min; }
    static T getMax() { return data().max; }
    static T getInc() { return data().inc; }
};

}  // namespace inviwo
