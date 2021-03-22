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

#include <modules/basegl/algorithm/volumenormalization.h>

#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

VolumeNormalization::VolumeNormalization()
    : shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, utilgl::findShaderResource("volumenormalization.frag")}},
              Shader::Build::No)
    , fbo_()
    , needsCompilation_(false) {
    shader_.getFragmentShaderObject()->addShaderDefine("NORMALIZE_CHANNEL_0");
    shader_.build();
}

void VolumeNormalization::setNormalizeChannel(const size_t channel, const bool normalize) {
    needsCompilation_ = true;

    if (normalize) {
        shader_.getFragmentShaderObject()->addShaderDefine(defines_[channel]);
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine(defines_[channel]);
    }
}

void VolumeNormalization::setNormalizeChannels(bvec4 normalize) {
    setNormalizeChannel(0, normalize[0]);
    setNormalizeChannel(1, normalize[1]);
    setNormalizeChannel(2, normalize[2]);
    setNormalizeChannel(3, normalize[3]);
}

void VolumeNormalization::reset() { setNormalizeChannels({true, false, false, false}); }

std::shared_ptr<Volume> VolumeNormalization::normalize(const Volume& volume) {
    auto outVolume = std::make_shared<Volume>(volume.getDimensions(), volume.getDataFormat(),
                                              volume.getSwizzleMask(), volume.getInterpolation(),
                                              volume.getWrapping());
    outVolume->setModelMatrix(volume.getModelMatrix());
    outVolume->setWorldMatrix(volume.getWorldMatrix());
    outVolume->copyMetaDataFrom(volume);

    if (needsCompilation_) {
        needsCompilation_ = false;
        shader_.build();
    }

    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, volume, "volume");

    const size3_t dim{volume.getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    VolumeGL* outVolumeGL = outVolume->getEditableRepresentation<VolumeGL>();

    fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();

    outVolume->dataMap_.dataRange = outVolume->dataMap_.valueRange = dvec2{0, 1};

    return outVolume;
}

}  // namespace inviwo
