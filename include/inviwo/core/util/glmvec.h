/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <warn/pop>

// extracted from <glm/gtx/std_based_type.hpp> to avoid including glm.hpp
namespace glm {
typedef vec<1, std::size_t, defaultp> size1;
typedef vec<2, std::size_t, defaultp> size2;
typedef vec<3, std::size_t, defaultp> size3;
typedef vec<4, std::size_t, defaultp> size4;
typedef vec<1, std::size_t, defaultp> size1_t;
typedef vec<2, std::size_t, defaultp> size2_t;
typedef vec<3, std::size_t, defaultp> size3_t;
typedef vec<4, std::size_t, defaultp> size4_t;
}  // namespace glm

namespace inviwo {

using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using ivec4 = glm::ivec4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using dvec2 = glm::dvec2;
using dvec3 = glm::dvec3;
using dvec4 = glm::dvec4;
using bvec2 = glm::bvec2;
using bvec3 = glm::bvec3;
using bvec4 = glm::bvec4;
using uvec2 = glm::uvec2;
using uvec3 = glm::uvec3;
using uvec4 = glm::uvec4;
using size2_t = glm::size2_t;
using size3_t = glm::size3_t;
using size4_t = glm::size4_t;
using i64vec2 = glm::i64vec2;
using i64vec3 = glm::i64vec3;
using i64vec4 = glm::i64vec4;
using u64vec2 = glm::u64vec2;
using u64vec3 = glm::u64vec3;
using u64vec4 = glm::u64vec4;

}  // namespace inviwo
