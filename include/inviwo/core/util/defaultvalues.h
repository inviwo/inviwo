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

#ifndef IVW_DEFAULTVALUES_H
#define IVW_DEFAULTVALUES_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glm.h>

#include <string>

namespace inviwo {

template <typename T>
struct InviwoDefaultData {
    std::string name;
    uvec2 dim;
    T val;
    T min;
    T max;
    T inc;
};

template <typename T>
struct InviwoDefaults {};

template <>
struct InviwoDefaults<int> {
    static InviwoDefaultData<int> get() { return {"Int", uvec2(1, 1), 0, -100, 100, 1}; }
};
template <>
struct InviwoDefaults<unsigned int> {
    static InviwoDefaultData<unsigned int> get() { return {"UInt", uvec2(1, 1), 0, 0, 100, 1}; }
};
template <>
struct InviwoDefaults<glm::i64> {
    static InviwoDefaultData<glm::i64> get() { return {"Int64", uvec2(1, 1), 0, 0, 1024, 1}; }
};
template <>
struct InviwoDefaults<size_t> {
    static InviwoDefaultData<size_t> get() { return {"Size_t", uvec2(1, 1), 0, 0, 100, 1}; }
};
template <>
struct InviwoDefaults<float> {
    static InviwoDefaultData<float> get() {
        return {"Float", uvec2(1, 1), 0.0f, 0.0f, 1.0f, 0.01f};
    }
};
template <>
struct InviwoDefaults<double> {
    static InviwoDefaultData<double> get() { return {"Double", uvec2(1, 1), 0.0, 0.0, 1.0, 0.01}; }
};

template <>
struct InviwoDefaults<std::string> {
    static InviwoDefaultData<std::string> get() { return {"String", uvec2(1, 1), "", "", "", ""}; }
};

template <>
struct InviwoDefaults<bool> {
    static InviwoDefaultData<bool> get() { return {"Bool", uvec2(1, 1), false, false, true, true}; }
};

template <>
struct InviwoDefaults<glm::dquat> {
    static InviwoDefaultData<glm::dquat> get() {
        return {"DoubleQuaternion",         uvec2(4, 1),
                glm::dquat(0., 0., 0., 0.), glm::dquat(-1., -1., -1., -1.),
                glm::dquat(1., 1., 1., 1.), glm::dquat(0.01, 0.01, 0.01, 0.01)};
    }
};
template <>
struct InviwoDefaults<glm::fquat> {
    static InviwoDefaultData<glm::fquat> get() {
        return {"FloatQuaternion",
                uvec2(4, 1),
                glm::fquat(0.f, 0.f, 0.f, 0.f),
                glm::fquat(-1.f, -1.f, -1.f, -1.f),
                glm::fquat(1.f, 1.f, 1.f, 1.f),
                glm::fquat(0.01f, 0.01f, 0.01f, 0.01f)};
    }
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct InviwoDefaults<glm::vec<L, T, Q>> {
    using type = glm::vec<L, T, Q>;
    static InviwoDefaultData<type> get() {
        const auto t = InviwoDefaults<T>::get();
        return {t.name + "Vec" + std::to_string(L),
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
    static InviwoDefaultData<type> get() {
        const auto t = InviwoDefaults<T>::get();
        return {t.name + "Mat" +
                    (C == R ? std::to_string(C) : std::to_string(C) + "x" + std::to_string(R)),
                uvec2(C, R),
                type{0} + t.val,
                type{0} + t.min,
                type{0} + t.max,
                type{0} + t.inc};
    }
};

template <glm::length_t L, glm::qualifier Q>
struct InviwoDefaults<glm::vec<L, size_t, Q>> {
    using type = glm::vec<L, size_t, Q>;
    static InviwoDefaultData<type> get() {
        const auto t = InviwoDefaults<size_t>::get();
        return {"IntSize" + std::to_string(L),
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
    static InviwoDefaultData<T> data() { return InviwoDefaults<T>::get(); };
    static std::string getName() { return data().name; }
    static uvec2 getDim() { return data().dim; }
    static T getVal() { return data().val; }
    static T getMin() { return data().min; }
    static T getMax() { return data().max; }
    static T getInc() { return data().inc; }
};

}  // namespace inviwo

#endif  // IVW_DEFAULTVALUES_H
