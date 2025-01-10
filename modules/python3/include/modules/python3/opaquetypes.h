/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <warn/pop>

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>

// This needs to be included where ever we might encounter any of these types.
// otherwise we might pick up on other caster, usually the list_caster from pybind11/stl.h
// which will create an ODR violation since we will have typecasters with different base types

// Need to declare vectors of glm types as opaque otherwise they will be copied back and
// forth to python

// Vecs
PYBIND11_MAKE_OPAQUE(std::vector<float>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<4, float>>)

PYBIND11_MAKE_OPAQUE(std::vector<double>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<4, double>>)

PYBIND11_MAKE_OPAQUE(std::vector<int>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<4, int>>)

PYBIND11_MAKE_OPAQUE(std::vector<unsigned int>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::vec<4, unsigned int>>)

// Mat Float
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, float>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, float>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, float>>)

// Mat Double
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, double>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, double>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, double>>)

// Mat int
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, int>>)

// Mat uint
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, unsigned int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, unsigned int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, unsigned int>>)
