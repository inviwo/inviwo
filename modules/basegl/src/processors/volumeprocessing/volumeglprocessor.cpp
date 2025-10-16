/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>

#include <inviwo/core/algorithm/markdown.h>                             // for operator""_help
#include <inviwo/core/datastructures/data.h>                            // for noData
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport, Vol...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/util/glmvec.h>                                    // for size3_t
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>                                // for glViewport, GLsizei
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shadertype.h>                           // for ShaderType, Shade...
#include <modules/opengl/shader/shaderutils.h>                          // for findShaderResource
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for multiDrawImagePla...
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/volume/volumegl.h>     // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>  // for bindAndSetUniforms
#include <modules/opengl/openglutils.h>

#include <algorithm>
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair
#include <span>

namespace inviwo {

namespace {

FrameBufferObject& getActiveFBO(Texture3D& texture, std::span<VolumeGLProcessor::FBO> fbos) {
    auto it = std::ranges::find_if(
        fbos, [&](const auto& fbo) { return fbo.textureId == texture.getID(); });
    if (it == fbos.end()) {
        it = std::ranges::min_element(
            fbos, [](const auto& a, const auto& b) { return a.lastUsed < b.lastUsed; });
        it->fbo.activate();
        it->fbo.attachColorTexture(&texture, 0);
        it->textureId = texture.getID();
    } else {
        it->fbo.activate();
    }
    it->lastUsed = std::chrono::high_resolution_clock::now();

    return it->fbo;
}

void detachFBOs(std::span<VolumeGLProcessor::FBO> fbos) {
    for (auto& fbo : fbos) {
        fbo.fbo.activate();
        fbo.fbo.detachAllTextures();
        fbo.textureId = 0;
    }
}

}  // namespace

VolumeGLProcessor::VolumeGLProcessor(std::shared_ptr<const ShaderResource> fragmentShaderResource,
                                     VolumeConfig config)
    : Processor{}
    , inport_{"inputVolume", "Input volume"_help}
    , outport_{"outputVolume", "Output volume"_help}
    , calculateDataRange_{"calculateDataRange_", "Calculate Data Range",
                          "Calculate and assign a new data range for the volume."_help, false}
    , dataRange_{"dataRange", "Data Range", true}
    , config_{std::move(config)}
    , volumes_{}
    , shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, fragmentShaderResource
                                          ? std::move(fragmentShaderResource)
                                          : utilgl::findShaderResource("volume_gpu.frag")}},
              Shader::Build::No)
    , fbos_{} {

    addPorts(inport_, outport_);
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

VolumeGLProcessor::VolumeGLProcessor(std::string_view fragmentShader, VolumeConfig config)
    : VolumeGLProcessor{utilgl::findShaderResource(fragmentShader), std::move(config)} {}

VolumeGLProcessor::~VolumeGLProcessor() = default;

void VolumeGLProcessor::initializeResources() {
    initializeShader(shader_);
    shader_.build();
}

void VolumeGLProcessor::process() {
    shader_.activate();

    TextureUnitContainer cont;

    const auto srcVolume = inport_.getData();
    auto config = srcVolume->config().updateFrom(config_);

    preProcess(cont, shader_, config);

    utilgl::bindAndSetUniforms(shader_, cont, *srcVolume, "volume");

    if (volumes_.setConfig(config) == VolumeReuseCache::Status::ClearedCache) {
        detachFBOs(fbos_);
    }
    auto dstVolume = volumes_.get();

    const size3_t dim{srcVolume->getDimensions()};

    // We always need to ask for an editable representation
    // this will invalidate any other representations
    auto* outVolumeGL = dstVolume->getEditableRepresentation<VolumeGL>();

    auto& fbo = getActiveFBO(*outVolumeGL->getTexture(), fbos_);

    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo.deactivate();

    if (calculateDataRange_) {
        if (!dataMinMaxGL_) {
            dataMinMaxGL_.emplace();
        }
        const auto [min, max] = dataMinMaxGL_->minMax(*dstVolume);
        auto dataRange = [&]() -> glm::dvec2 {
            switch (dstVolume->getDataFormat()->getComponents()) {
                case 1:
                    return {min.x, max.x};
                case 2:
                    return {glm::compMin(dvec2{min}), glm::compMax(dvec2{max})};
                case 3:
                    return {glm::compMin(dvec3{min}), glm::compMax(dvec3{max})};
                case 4:
                    [[fallthrough]];
                default:
                    return {glm::compMin(min), glm::compMax(max)};
            }
        }();

        if (dataRange.x == dataRange.y) {
            dataRange = DataMapper::defaultDataRangeFor(dstVolume->getDataFormat());
        }
        dstVolume->dataMap.dataRange = dataRange;
        dstVolume->dataMap.valueRange = dataRange;
    }

    dataRange_.updateFromVolume(dstVolume);
    dstVolume->dataMap.dataRange = dataRange_.getDataRange();
    dstVolume->dataMap.valueRange = dataRange_.getValueRange();

    postProcess(*dstVolume);
    dstVolume->discardHistograms();  // remove any old histograms;
    outport_.setData(dstVolume);
}

void VolumeGLProcessor::setFragmentShaderResource(
    std::shared_ptr<const ShaderResource> fragShaderResource) {
    if (auto* frag = shader_.getFragmentShaderObject()) {
        frag->setResource(std::move(fragShaderResource));
    }
}
std::shared_ptr<const ShaderResource> VolumeGLProcessor::getFragmentShaderResource() {
    if (const auto* frag = shader_.getFragmentShaderObject()) {
        return frag->getResource();
    }
    return {};
}

void VolumeGLProcessor::initializeShader(Shader&) {}

void VolumeGLProcessor::preProcess(TextureUnitContainer&, Shader&, VolumeConfig&) {}

void VolumeGLProcessor::postProcess(Volume&) {}

}  // namespace inviwo
