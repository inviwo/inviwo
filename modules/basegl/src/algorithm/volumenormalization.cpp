/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/data.h>                            // for noData
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for bvec4, size3_t
#include <inviwo/core/util/exception.h>
#include <modules/opengl/buffer/framebufferobject.h>  // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>              // for glViewport, GLsizei
#include <modules/opengl/shader/shader.h>             // for Shader, Shader::B...
#include <modules/opengl/shader/shaderobject.h>       // for ShaderObject
#include <modules/opengl/shader/shadertype.h>         // for ShaderType, Shade...
#include <modules/opengl/shader/shaderutils.h>        // for findShaderResource
#include <modules/opengl/texture/textureunit.h>       // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>      // for multiDrawImagePla...
#include <modules/opengl/volume/volumegl.h>           // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>        // for bindAndSetUniforms

#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair

#include <glm/vec3.hpp>  // for vec<>::(anonymous)

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
        throw RangeException(IVW_CONTEXT, "channel out of bounds ({})", channel);
    }
    normalizeChannel_[channel] = normalize;
}

void VolumeNormalization::setNormalizeChannels(bvec4 normalize) { normalizeChannel_ = normalize; }

void VolumeNormalization::reset() { normalizeChannel_ = bvec4{true}; }

std::shared_ptr<Volume> VolumeNormalization::normalize(const Volume& volume) {
    std::shared_ptr<Volume> outVolume;

    outVolume = std::make_shared<Volume>(volume, noData);
    outVolume->setDataFormat(
        DataFormatBase::get(NumericType::Float, volume.getDataFormat()->getComponents(), 32));

    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, volume, "volume");
    shader_.setUniform("normalizeChannel", normalizeChannel_);

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
