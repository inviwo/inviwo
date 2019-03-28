/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/util/glm.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

// Manually expand PYBIND11_MAKE_OPAQUE because macros and commas in template arguments mix poorly.
namespace pybind11 {
namespace detail {
using namespace inviwo;
// clang-format off
    template<> class type_caster<Vector<2, float>> : public type_caster_base<Vector<2, float>> { };
    template<> class type_caster<typename util::glmtype<float, 2, 2>::type> : public type_caster_base<typename util::glmtype<float, 2, 2>::type> { };
    template<> class type_caster<typename util::glmtype<float, 2, 3>::type> : public type_caster_base<typename util::glmtype<float, 2, 3>::type> { };
    template<> class type_caster<typename util::glmtype<float, 2, 4>::type> : public type_caster_base<typename util::glmtype<float, 2, 4>::type> { };
    template<> class type_caster<Vector<3, float>> : public type_caster_base<Vector<3, float>> { };
    template<> class type_caster<typename util::glmtype<float, 3, 2>::type> : public type_caster_base<typename util::glmtype<float, 3, 2>::type> { };
    template<> class type_caster<typename util::glmtype<float, 3, 3>::type> : public type_caster_base<typename util::glmtype<float, 3, 3>::type> { };
    template<> class type_caster<typename util::glmtype<float, 3, 4>::type> : public type_caster_base<typename util::glmtype<float, 3, 4>::type> { };
    template<> class type_caster<Vector<4, float>> : public type_caster_base<Vector<4, float>> { };
    template<> class type_caster<typename util::glmtype<float, 4, 2>::type> : public type_caster_base<typename util::glmtype<float, 4, 2>::type> { };
    template<> class type_caster<typename util::glmtype<float, 4, 3>::type> : public type_caster_base<typename util::glmtype<float, 4, 3>::type> { };
    template<> class type_caster<typename util::glmtype<float, 4, 4>::type> : public type_caster_base<typename util::glmtype<float, 4, 4>::type> { };
    template<> class type_caster<Vector<2, double>> : public type_caster_base<Vector<2, double>> { };
    template<> class type_caster<typename util::glmtype<double, 2, 2>::type> : public type_caster_base<typename util::glmtype<double, 2, 2>::type> { };
    template<> class type_caster<typename util::glmtype<double, 2, 3>::type> : public type_caster_base<typename util::glmtype<double, 2, 3>::type> { };
    template<> class type_caster<typename util::glmtype<double, 2, 4>::type> : public type_caster_base<typename util::glmtype<double, 2, 4>::type> { };
    template<> class type_caster<Vector<3, double>> : public type_caster_base<Vector<3, double>> { };
    template<> class type_caster<typename util::glmtype<double, 3, 2>::type> : public type_caster_base<typename util::glmtype<double, 3, 2>::type> { };
    template<> class type_caster<typename util::glmtype<double, 3, 3>::type> : public type_caster_base<typename util::glmtype<double, 3, 3>::type> { };
    template<> class type_caster<typename util::glmtype<double, 3, 4>::type> : public type_caster_base<typename util::glmtype<double, 3, 4>::type> { };
    template<> class type_caster<Vector<4, double>> : public type_caster_base<Vector<4, double>> { };
    template<> class type_caster<typename util::glmtype<double, 4, 2>::type> : public type_caster_base<typename util::glmtype<double, 4, 2>::type> { };
    template<> class type_caster<typename util::glmtype<double, 4, 3>::type> : public type_caster_base<typename util::glmtype<double, 4, 3>::type> { };
    template<> class type_caster<typename util::glmtype<double, 4, 4>::type> : public type_caster_base<typename util::glmtype<double, 4, 4>::type> { };
    template<> class type_caster<Vector<2, int>> : public type_caster_base<Vector<2, int>> { };
    template<> class type_caster<typename util::glmtype<int, 2, 2>::type> : public type_caster_base<typename util::glmtype<int, 2, 2>::type> { };
    template<> class type_caster<typename util::glmtype<int, 2, 3>::type> : public type_caster_base<typename util::glmtype<int, 2, 3>::type> { };
    template<> class type_caster<typename util::glmtype<int, 2, 4>::type> : public type_caster_base<typename util::glmtype<int, 2, 4>::type> { };
    template<> class type_caster<Vector<3, int>> : public type_caster_base<Vector<3, int>> { };
    template<> class type_caster<typename util::glmtype<int, 3, 2>::type> : public type_caster_base<typename util::glmtype<int, 3, 2>::type> { };
    template<> class type_caster<typename util::glmtype<int, 3, 3>::type> : public type_caster_base<typename util::glmtype<int, 3, 3>::type> { };
    template<> class type_caster<typename util::glmtype<int, 3, 4>::type> : public type_caster_base<typename util::glmtype<int, 3, 4>::type> { };
    template<> class type_caster<Vector<4, int>> : public type_caster_base<Vector<4, int>> { };
    template<> class type_caster<typename util::glmtype<int, 4, 2>::type> : public type_caster_base<typename util::glmtype<int, 4, 2>::type> { };
    template<> class type_caster<typename util::glmtype<int, 4, 3>::type> : public type_caster_base<typename util::glmtype<int, 4, 3>::type> { };
    template<> class type_caster<typename util::glmtype<int, 4, 4>::type> : public type_caster_base<typename util::glmtype<int, 4, 4>::type> { };
    template<> class type_caster<Vector<2, unsigned int>> : public type_caster_base<Vector<2, unsigned int>> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 2, 2>::type> : public type_caster_base<typename util::glmtype<unsigned int, 2, 2>::type> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 2, 3>::type> : public type_caster_base<typename util::glmtype<unsigned int, 2, 3>::type> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 2, 4>::type> : public type_caster_base<typename util::glmtype<unsigned int, 2, 4>::type> { };
    template<> class type_caster<Vector<3, unsigned int>> : public type_caster_base<Vector<3, unsigned int>> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 3, 2>::type> : public type_caster_base<typename util::glmtype<unsigned int, 3, 2>::type> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 3, 3>::type> : public type_caster_base<typename util::glmtype<unsigned int, 3, 3>::type> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 3, 4>::type> : public type_caster_base<typename util::glmtype<unsigned int, 3, 4>::type> { };
    template<> class type_caster<Vector<4, unsigned int>> : public type_caster_base<Vector<4, unsigned int>> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 4, 2>::type> : public type_caster_base<typename util::glmtype<unsigned int, 4, 2>::type> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 4, 3>::type> : public type_caster_base<typename util::glmtype<unsigned int, 4, 3>::type> { };
    template<> class type_caster<typename util::glmtype<unsigned int, 4, 4>::type> : public type_caster_base<typename util::glmtype<unsigned int, 4, 4>::type> { };
    template<> class type_caster<Vector<2, size_t>> : public type_caster_base<Vector<2, size_t>> { };
    template<> class type_caster<Vector<3, size_t>> : public type_caster_base<Vector<3, size_t>> { };
    template<> class type_caster<Vector<4, size_t>> : public type_caster_base<Vector<4, size_t>> { };

    template<> class type_caster<std::vector<Vector<2, float>>> : public type_caster_base<std::vector<Vector<2, float>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 2, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 2, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 2, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 2, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 2, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 2, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<3, float>>> : public type_caster_base<std::vector<Vector<3, float>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 3, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 3, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 3, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 3, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 3, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 3, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<4, float>>> : public type_caster_base<std::vector<Vector<4, float>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 4, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 4, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 4, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 4, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<float, 4, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<float, 4, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<2, double>>> : public type_caster_base<std::vector<Vector<2, double>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 2, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 2, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 2, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 2, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 2, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 2, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<3, double>>> : public type_caster_base<std::vector<Vector<3, double>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 3, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 3, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 3, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 3, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 3, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 3, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<4, double>>> : public type_caster_base<std::vector<Vector<4, double>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 4, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 4, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 4, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 4, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<double, 4, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<double, 4, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<2, int>>> : public type_caster_base<std::vector<Vector<2, int>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 2, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 2, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 2, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 2, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 2, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 2, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<3, int>>> : public type_caster_base<std::vector<Vector<3, int>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 3, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 3, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 3, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 3, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 3, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 3, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<4, int>>> : public type_caster_base<std::vector<Vector<4, int>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 4, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 4, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 4, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 4, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<int, 4, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<int, 4, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<2, unsigned int>>> : public type_caster_base<std::vector<Vector<2, unsigned int>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 2, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 2, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 2, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 2, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 2, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 2, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<3, unsigned int>>> : public type_caster_base<std::vector<Vector<3, unsigned int>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 3, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 3, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 3, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 3, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 3, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 3, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<4, unsigned int>>> : public type_caster_base<std::vector<Vector<4, unsigned int>>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 4, 2>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 4, 2>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 4, 3>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 4, 3>::type>> { };
    template<> class type_caster<std::vector<typename util::glmtype<unsigned int, 4, 4>::type>> : public type_caster_base<std::vector<typename util::glmtype<unsigned int, 4, 4>::type>> { };
    template<> class type_caster<std::vector<Vector<2, size_t>>> : public type_caster_base<std::vector<Vector<2, size_t>>> { };
    template<> class type_caster<std::vector<Vector<3, size_t>>> : public type_caster_base<std::vector<Vector<3, size_t>>> { };
    template<> class type_caster<std::vector<Vector<4, size_t>>> : public type_caster_base<std::vector<Vector<4, size_t>>> { };
// clang-format on
}  // namespace detail
}  // namespace pybind11

namespace inviwo {
void exposeGLMTypes(pybind11::module &m);
}
