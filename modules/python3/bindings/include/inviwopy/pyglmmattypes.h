/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <warn/pop>

#include <vector>

// Need to declare vectors of glm types as opaque otherwise they will be copied back and
// forth to python
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, float>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, float>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, float>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, float>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, double>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, double>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, double>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, double>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<2, 4, unsigned int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<3, 4, unsigned int>>)

PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 2, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 3, unsigned int>>)
PYBIND11_MAKE_OPAQUE(std::vector<glm::mat<4, 4, unsigned int>>)

namespace inviwo {

void exposeGLMMatTypes(pybind11::module& m);

}
