/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/basegl/util/periodicitygl.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/glm.h>

#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

PeriodicityGL::PeriodicityGL()
    : periodicity{"periodicity", "Enable Periodicity", false}
    , basis{"basis", "Basis", util::ordinalSymmetricVector(mat4{1.0f}, util::filled<mat4>(100.0f))}
    , shift{"shift", "Shift", util::ordinalSymmetricVector(vec3{0.0f}, 1.0f)}
    , repeat{"repeat", "Repeat", util::ordinalCount(ivec3{1}, 10).setMin(ivec3{1})}
    , duplicateCutoff{"duplicateCutoff", "Duplicate Cutoff",
                      util::ordinalLength(0.0f, 1.0f).set(InvalidationLevel::InvalidResources)} {

    periodicity.addProperties(shift, repeat, duplicateCutoff, basis);
    periodicity.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
}

void PeriodicityGL::addDefines(Shader& shader) const {
    shader[ShaderType::Vertex]->setShaderDefine("ENABLE_PERIODICITY", periodicity.isChecked());
    shader[ShaderType::Geometry]->setShaderDefine("ENABLE_PERIODICITY", periodicity.isChecked());
    shader[ShaderType::Geometry]->setShaderDefine(
        "ENABLE_DUPLICATE", periodicity.isChecked() && duplicateCutoff.get() > 0.0f);
}
void PeriodicityGL::setUniforms(Shader& shader) const {
    utilgl::setUniforms(shader, basis, repeat, shift, duplicateCutoff);
}

size_t PeriodicityGL::instances() const { return static_cast<size_t>(glm::compMul(repeat.get())); }

std::function<std::optional<mat4>()> PeriodicityGL::boundingBox(MeshFlatMultiInport& port) const {
    return [this, port = &port]() -> std::optional<mat4> {
        if (port->hasData()) {
            if (periodicity) {
                const auto d2w = port->getData()->getCoordinateTransformer().getDataToWorldMatrix();
                const auto scale = glm::scale(vec3(repeat.get()));
                return d2w * scale * basis.get();

            } else {
                return util::boundingBox(port->getVectorData());
            }
        } else {
            return std::nullopt;
        }
    };
}

}  // namespace inviwo
