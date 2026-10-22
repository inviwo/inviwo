/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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
using size1 = vec<1, std::size_t, defaultp>;
using size2 = vec<2, std::size_t, defaultp>;
using size3 = vec<3, std::size_t, defaultp>;
using size4 = vec<4, std::size_t, defaultp>;
using size1_t = vec<1, std::size_t, defaultp>;
using size2_t = vec<2, std::size_t, defaultp>;
using size3_t = vec<3, std::size_t, defaultp>;
using size4_t = vec<4, std::size_t, defaultp>;
}  // namespace glm

namespace inviwo {

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
using glm::bvec2;
using glm::bvec3;
using glm::bvec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::size2_t;
using glm::size3_t;
using glm::size4_t;
using glm::i64vec2;
using glm::i64vec3;
using glm::i64vec4;
using glm::u64vec2;
using glm::u64vec3;
using glm::u64vec4;

}  // namespace inviwo
