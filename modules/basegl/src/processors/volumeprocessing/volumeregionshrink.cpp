/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumeregionshrink.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/volumeport.h>                               // for VolumeOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for size3_t
#include <inviwo/core/util/raiiutils.h>                                 // for KeepTrueWhileInScope
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>                                // for glDrawBuffer, glV...
#include <modules/opengl/openglutils.h>                                 // for Activate
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shadermanager.h>                        // for ShaderManager
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/shader/shaderresource.h>                       // for StringShaderResource
#include <modules/opengl/shader/shadertype.h>                           // for ShaderType, Shade...
#include <modules/opengl/shader/shaderutils.h>                          // for findShaderResource
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for multiDrawImagePla...
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms

#include <cstddef>        // for size_t
#include <functional>     // for __base
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair, swap

#include <fmt/core.h>    // for format
#include <glm/vec3.hpp>  // for operator!=, vec<>...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionShrink::processorInfo_{
    "org.inviwo.VolumeRegionShrink",  // Class identifier
    "Volume Region Shrink",           // Display name
    "Volume Operation",               // Category
    CodeState::Stable,                // Code state
    Tags::GL,                         // Tags
    R"(Shrinks regions of identical values. The processor will assign a FillValue to
    each border voxel in each iteration. A voxel is considered on the border if the
    value of any of the 26 closest neighbors is different. The procedure is repeated
    number of iterations times.)"_unindentHelp};

const ProcessorInfo VolumeRegionShrink::getProcessorInfo() const { return processorInfo_; }

namespace {

constexpr std::string_view fragStr = R"(
#include "utils/sampler3d.glsl"
#include "utils/structs.glsl"

layout(location = 0) out {0}vec4 FragData0;

uniform {0}sampler3D volume;
uniform VolumeParameters volumeParameters;
uniform VolumeParameters dstParameters;
uniform {0}vec4 fillValue;

in vec4 texCoord_;

void main() {{
    {0}vec4 value = texture(volume, texCoord_.xyz);

    bool border = false;
    for (int z = -1; z < 1; z++) {{
        for (int y = -1; y < 1; y++) {{
            for (int x = -1; x < 1; x++) {{
                border = border || value != texture(volume,
                                            texCoord_.xyz + vec3(x,y,z) *
                                            volumeParameters.reciprocalDimensions);
            }}
        }}
    }}
    FragData0 = mix(value, fillValue, bvec4(border));
}}
)";

}

VolumeRegionShrink::VolumeRegionShrink()
    : Processor()
    , inport_{"inputVolume"}
    , outport_{"outputVolume"}
    , iterations_{"iterations", "Iterations", 3, 0, 25}
    , dataRange_{"dataRange",
                 "Data range",
                 0.,
                 255.0,
                 -DataFloat64::max(),
                 DataFloat64::max(),
                 0.0,
                 0.0,
                 InvalidationLevel::Valid,
                 PropertySemantics::Text}
    , valueRange_{"valueRange",
                  "Value range",
                  0.,
                  255.0,
                  -DataFloat64::max(),
                  DataFloat64::max(),
                  0.0,
                  0.0,
                  InvalidationLevel::Valid,
                  PropertySemantics::Text}
    , fillValue_{"fillValue",
                 "Fill Value",
                 0,
                 0,
                 DataInt32::max(),
                 1,
                 InvalidationLevel::InvalidOutput,
                 PropertySemantics::Text}
    , volumeNumericType_{""}
    , fragShader_{std::make_shared<StringShaderResource>("VolumeRegionShrink.frag",
                                                         fmt::format(fragStr, volumeNumericType_))}
    , shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, fragShader_}},
              Shader::Build::No) {

    addPort(inport_);
    addPort(outport_);

    dataRange_.setReadOnly(true);
    valueRange_.setReadOnly(true);

    addProperties(iterations_, dataRange_, valueRange_, fillValue_);

    shader_.onReload([this]() {
        if (blockShaderReload_) return;
        invalidate(InvalidationLevel::InvalidResources);
    });
    ShaderManager::getPtr()->addShaderResource(fragShader_);
}

void VolumeRegionShrink::process() {
    auto volume = inport_.getData();

    if (iterations_ == 0) {
        outport_.setData(volume);
        return;
    }

    auto* vf = volume->getDataFormat();
    std::string volumeNumericType = "";
    if (vf->getPrecision() == 32 && vf->getNumericType() == NumericType::SignedInteger) {
        volumeNumericType = "i";
    } else if (vf->getPrecision() == 32 && vf->getNumericType() == NumericType::UnsignedInteger) {
        volumeNumericType = "u";
    }
    if (volumeNumericType != volumeNumericType_) {
        volumeNumericType_ = volumeNumericType;
        util::KeepTrueWhileInScope block(&blockShaderReload_);
        initializeResources();
    }

    if (!out_[0] || out_[0]->getDataFormat() != volume->getDataFormat() ||
        out_[0]->getDimensions() != volume->getDimensions()) {
        out_[0] = std::shared_ptr<Volume>(volume->clone());
    }

    dataRange_.set(volume->dataMap.dataRange);
    valueRange_.set(volume->dataMap.valueRange);

    out_[0]->setModelMatrix(volume->getModelMatrix());
    out_[0]->setWorldMatrix(volume->getWorldMatrix());
    out_[0]->setSwizzleMask(volume->getSwizzleMask());
    out_[0]->setWrapping(volume->getWrapping());
    out_[0]->setInterpolation(volume->getInterpolation());
    out_[0]->copyMetaDataFrom(*volume);
    out_[0]->dataMap = volume->dataMap;
    out_[0]->dataMap.dataRange.x =
        std::min(out_[0]->dataMap.dataRange.x, static_cast<double>(fillValue_));
    out_[0]->dataMap.dataRange.y =
        std::max(out_[0]->dataMap.dataRange.y, static_cast<double>(fillValue_));
    out_[0]->dataMap.valueRange = out_[0]->dataMap.dataRange;

    const size3_t dim{volume->getDimensions()};
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    fbo_.activate();
    VolumeGL* outGL0 = out_[0]->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(outGL0->getTexture().get(), 0);

    utilgl::Activate as{&shader_};

    if (volumeNumericType_ == "i") {
        shader_.setUniform("fillValue", ivec4{fillValue_.get()});
    } else if (volumeNumericType_ == "u") {
        shader_.setUniform("fillValue", uvec4{static_cast<std::uint32_t>(fillValue_.get())});
    } else {
        const auto fill =
            static_cast<double>(fillValue_) / (1.0 + out_[0]->getDataFormat()->getMax());
        shader_.setUniform("fillValue", vec4{static_cast<float>(fill)});
    }

    // Iteration 1
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        TextureUnitContainer cont;
        utilgl::bindAndSetUniforms(shader_, cont, *volume, "volume");
        utilgl::setShaderUniforms(shader_, *out_[0], "dstParameters");
        utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));
    }
    if (iterations_ == 1) {
        FrameBufferObject::deactivateFBO();
        outport_.setData(out_[0]);
        out_[1].reset();
        return;
    }

    if (!out_[1] || out_[1]->getDataFormat() != volume->getDataFormat() ||
        out_[1]->getDimensions() != volume->getDimensions()) {
        out_[1] = std::shared_ptr<Volume>(volume->clone());
    }

    out_[1]->setModelMatrix(volume->getModelMatrix());
    out_[1]->setWorldMatrix(volume->getWorldMatrix());
    out_[1]->setSwizzleMask(volume->getSwizzleMask());
    out_[1]->setWrapping(volume->getWrapping());
    out_[1]->setInterpolation(volume->getInterpolation());
    out_[1]->copyMetaDataFrom(*volume);
    out_[1]->dataMap = volume->dataMap;
    out_[1]->dataMap.dataRange.x =
        std::min(out_[1]->dataMap.dataRange.x, static_cast<double>(fillValue_));
    out_[1]->dataMap.dataRange.y =
        std::max(out_[1]->dataMap.dataRange.y, static_cast<double>(fillValue_));
    out_[1]->dataMap.valueRange = out_[1]->dataMap.dataRange;

    VolumeGL* outGL1 = out_[1]->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(outGL1->getTexture().get(), 1);

    size_t src = 1;
    size_t dst = 0;
    for (int i = 1; i < iterations_; ++i) {
        std::swap(src, dst);
        glDrawBuffer(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + dst));
        TextureUnitContainer cont;
        utilgl::bindAndSetUniforms(shader_, cont, *out_[src], "volume");
        utilgl::setShaderUniforms(shader_, *out_[dst], "dstParameters");
        utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));
    }

    FrameBufferObject::deactivateFBO();
    out_[dst]->discardHistograms();
    outport_.setData(out_[dst]);
}

void VolumeRegionShrink::initializeResources() {
    fragShader_->setSource(fmt::format(fragStr, volumeNumericType_));
    shader_.getFragmentShaderObject()->clearOutDeclarations();
    shader_.build();
}

}  // namespace inviwo
