/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_SIMPLEMESHCREATOR_H
#define IVW_SIMPLEMESHCREATOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>

namespace inviwo {

class IVW_CORE_API SimpleMeshCreator {
public:
    static std::shared_ptr<SimpleMesh> rectangularPrism(vec3 posLlf, vec3 posUrb, vec3 texCoordLlf,
                                                        vec3 texCoordUrb, vec4 colorLlf,
                                                        vec4 colorUrb);
    static std::shared_ptr<SimpleMesh> parallelepiped(glm::vec3 pos, glm::vec3 p1, glm::vec3 p2,
                                                      glm::vec3 p3, glm::vec3 tex, glm::vec3 t1,
                                                      glm::vec3 t2, glm::vec3 t3, glm::vec4 col,
                                                      glm::vec4 c1, glm::vec4 c2, glm::vec4 c3);

    static std::shared_ptr<SimpleMesh> rectangle(glm::vec3 posLl, glm::vec3 posUr);

    static std::shared_ptr<SimpleMesh> sphere(float radius, unsigned int numLoops,
                                              unsigned int segmentsPerLoop);
    static std::shared_ptr<SimpleMesh> sphere(float radius, unsigned int numLoops,
                                              unsigned int segmentsPerLoop, vec4 color);

    // create a plane centered at pos with normal pointing toward z
    // given extent and mesh resolution
    static std::shared_ptr<SimpleMesh> plane(glm::vec3 pos, glm::vec2 extent, unsigned int meshResX,
                                             unsigned int meshResY);
};

}  // namespace

#endif  // IVW_SIMPLEMESHCREATOR_H
