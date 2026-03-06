/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/basegl/algorithm/volumenormalization.h>

#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/volume/volumeutils.h>

#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <glm/vec3.hpp>

namespace inviwo {

VolumeNormalization::VolumeNormalization()
    : shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, utilgl::findShaderResource("volumenormalization.frag")}},
              Shader::Build::Yes)
    , fbo_()
    , normalizeChannel_(true) {}

void VolumeNormalization::setNormalizeChannel(const size_t channel, const bool normalize) {
    if (channel >= 4) {
        throw RangeException(SourceContext{}, "channel out of bounds ({})", channel);
    }
    normalizeChannel_[channel] = normalize;
}

void VolumeNormalization::setNormalizeChannels(bvec4 normalize) { normalizeChannel_ = normalize; }

void VolumeNormalization::reset() { normalizeChannel_ = bvec4{true}; }

std::shared_ptr<Volume> VolumeNormalization::normalize(const Volume& volume) {
    const size3_t dims{volume.getDimensions()};

    auto outVolume = std::make_shared<Volume>(
        volume, noData,
        VolumeConfig{.format = DataFormatBase::get(NumericType::Float,
                                                   volume.getDataFormat()->getComponents(), 32)});

    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, volume, "volume");
    shader_.setUniform("normalizeChannel", normalizeChannel_);

    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dims.x), static_cast<GLsizei>(dims.y));

    VolumeGL* outVolumeGL = outVolume->getEditableRepresentation<VolumeGL>();

    const auto drawBuff = fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);
    glDrawBuffer(drawBuff);
    utilgl::multiDrawImagePlaneRect(static_cast<int>(dims.z));

    shader_.deactivate();
    fbo_.deactivate();

    outVolume->dataMap.dataRange = outVolume->dataMap.valueRange = dvec2{0, 1};

    return outVolume;
}

}  // namespace inviwo
