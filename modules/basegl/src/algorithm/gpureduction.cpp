/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/basegl/algorithm/gpureduction.h>

#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

GPUReduction::GPUReduction()
    : shader_({{ShaderType::Compute, utilgl::findShaderResource("reduction/reduction.comp")}},
              Shader::Build::Yes)
    , activeReductionOperator_(ReductionOperator::None) {}

std::shared_ptr<Volume> GPUReduction::reduce(std::shared_ptr<const Volume> volume,
                                             const ReductionOperator op) {
    setReductionOperator(op);

    shader_.activate();

    const auto dimensions = volume->getDimensions();

    auto input = std::shared_ptr<Volume>(volume->clone());
    auto dataFormat = volume->getDataFormat();
    std::shared_ptr<Volume> output;

    auto m = static_cast<GLuint>(dimensions.x) / GLuint{2};
    auto n = static_cast<GLuint>(dimensions.y) / GLuint{2};
    auto o = static_cast<GLuint>(dimensions.z) / GLuint{2};

    while (true) {
        /*
         * Bind textures and set uniforms.
         */
        TextureUnitContainer cont;
        utilgl::bindAndSetUniforms(shader_, cont, *input, "inputTexture");
        shader_.setUniform("outputTexture", 0);

        /*
         * Update output texture.
         */
        output = std::make_shared<Volume>(size3_t{m, n, o}, dataFormat);
        auto outputGL = output->getEditableRepresentation<VolumeGL>();
        outputGL->setWrapping({Wrapping::Clamp, Wrapping::Clamp, Wrapping::Clamp});

        glActiveTexture(GL_TEXTURE0);

        auto texture = outputGL->getTexture();
        auto texHandle = texture->getID();
        glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                           texture->getInternalFormat());

        outputGL->setSwizzleMask(swizzlemasks::rgba);

        outputGL->getTexture()->bind();

        /*
         * Run reduction.
         */
        gpuDispatch(m, n, o);

        /*
         * When we have reached an output texture size of 1, the reduction is complete.
         */
        if (m == 1 && n == 1 && o == 1) break;

        /*
         * Swap textures.
         */
        std::swap(input, output);

        /*
         * Update output texture size
         */
        m = std::max(m / 2, GLuint{1});
        n = std::max(n / 2, GLuint{1});
        o = std::max(o / 2, GLuint{1});
    }

    shader_.deactivate();

    return output;
}

dvec4 GPUReduction::reduce_v(std::shared_ptr<const Volume> volume, const ReductionOperator op) {
    auto res = reduce(volume, op);

    return res->getRepresentation<VolumeRAM>()->getAsDVec4(size3_t{0, 0, 0});
}

void GPUReduction::setReductionOperator(ReductionOperator op) {
    if (op == activeReductionOperator_) return;

    auto computeShader = shader_.getShaderObject(ShaderType::Compute);
    computeShader->removeShaderDefine(StrBuffer{"OPERATOR {}", activeReductionOperator_});

    activeReductionOperator_ = op;

    computeShader->addShaderDefine(StrBuffer{"OPERATOR {}", activeReductionOperator_});

    shader_.build();
}

void GPUReduction::gpuDispatch(const GLuint x, const GLuint y, const GLuint z) {
    glDispatchCompute(x, y, z);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
}  // namespace inviwo
