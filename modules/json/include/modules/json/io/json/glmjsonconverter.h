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

#include <inviwo/core/common/inviwo.h>
#include <nlohmann/json.hpp>

namespace glm {

template <glm::length_t L, typename T, glm::qualifier Q>
void from_json(const nlohmann::json& j, glm::vec<L, T, Q>& v) {
    for (glm::length_t i = 0; i < L; ++i) {
        v[i] = j[i].get<T>();
    }
}

template <typename T, glm::qualifier Q>
void from_json(const nlohmann::json& j, glm::qua<T, Q>& v) {
    for (glm::length_t i = 0; i < glm::qua<T, Q>::length(); ++i) {
        v[i] = j[i].get<T>();
    }
}

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
void from_json(const nlohmann::json& js, glm::mat<C, R, T, Q>& v) {
    for (glm::length_t i = 0; i < C; ++i) {
        for (glm::length_t j = 0; j < R; ++j) {
            v[i][j] = js[i * C + j].get<T>();
        }
    }
}

template <glm::length_t L, typename T, glm::qualifier Q>
void to_json(nlohmann::json& j, const glm::vec<L, T, Q>& v) {
    for (glm::length_t i = 0; i < L; ++i) {
        j.push_back(v[i]);
    }
}

template <typename T, glm::qualifier Q>
void to_json(nlohmann::json& j, const glm::qua<T, Q>& v) {
    for (glm::length_t i = 0; i < glm::qua<T, Q>::length(); ++i) {
        j.push_back(v[i]);
    }
}

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
void to_json(nlohmann::json& js, const glm::mat<C, R, T, Q>& v) {
    for (glm::length_t i = 0; i < C; ++i) {
        for (glm::length_t j = 0; j < R; ++j) js.push_back(v[i][j]);
    }
}

}  // namespace glm
