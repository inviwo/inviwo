/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/datageneration/lorenzsystem.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/volumeport.h>                               // for VolumeOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                      // for FloatMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for OrdinalProperty
#include <inviwo/core/util/formats.h>                                   // for DataVec3Float32
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t, dvec2
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>                                // for GLenum, GLsizei
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/shader/shaderutils.h>                          // for setShaderUniforms
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for multiDrawImagePla...
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms

#include <array>          // for array
#include <functional>     // for __base
#include <memory>         // for shared_ptr, make_...
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <glm/detail/qualifier.hpp>  // for tvec2
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <glm/vec3.hpp>              // for operator-, vec

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LorenzSystem::processorInfo_{
    "org.inviwo.LorenzSystem",  // Class identifier
    "Lorenz System",            // Display name
    "Data Creation",            // Category
    CodeState::Stable,          // Code state
    "GL, Generator",            // Tags
    R"(Calculates the velocity, curl and div, of a lorenz system
    The velocity is given by
        
        sigma * (y - x)
        x * (rho - z) - y
        x * y - beta * z
        
    the curl is given by:
        
        2 * x
        -y
        rho - z - sigma
        
    and the div:
        
        -1 - sigma - beta
        
    )"_unindentHelp};
const ProcessorInfo& LorenzSystem::getProcessorInfo() const { return processorInfo_; }

LorenzSystem::LorenzSystem()
    : Processor()
    , velocityOutport_("outport")
    , curlOutport_("curl")
    , divOutport_("divergence")

    , size_("size", "Volume size", size3_t(32, 32, 32), size3_t(2, 2, 2), size3_t(1024, 1024, 1024))
    , xRange_("xRange", "X Range", -20, 20, -100, 100)
    , yRange_("yRange", "Y Range", -30, 30, -100, 100)
    , zRange_("zRange", "Z Range", 0, 50, 0, 100)
    , rho_("rho", "ρ Value", 28, 0, 100)
    , sigma_("sigma", "σ Value", 10, 0, 100)
    , beta_("beta", "β Value", 8.0f / 3.0f, 0, 100)
    , shader_("volume_gpu.vert", "volume_gpu.geom", "lorenzsystem.frag")
    , velocityVolumes_{}
    , curlVolumes_{}
    , divVolumes_{}
    , fbo_() {

    addPorts(velocityOutport_, curlOutport_, divOutport_);

    addProperties(size_, xRange_, yRange_, zRange_, rho_, sigma_, beta_);

    shader_.onReload([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

LorenzSystem::~LorenzSystem() = default;

void LorenzSystem::process() {
    const size3_t dims{size_.get()};

    auto velocityVolume = velocityVolumes_({.dimensions = dims, .format = DataVec3Float32::get()});
    auto curlVolume = curlVolumes_({.dimensions = dims, .format = DataVec3Float32::get()});
    auto divVolume = divVolumes_({.dimensions = dims, .format = DataFloat32::get()});

    // Basis and offset
    const std::array<vec3, 4> corners{{{xRange_.get().x, yRange_.get().x, zRange_.get().x},
                                       {xRange_.get().y, yRange_.get().x, zRange_.get().x},
                                       {xRange_.get().x, yRange_.get().y, zRange_.get().x},
                                       {xRange_.get().x, yRange_.get().x, zRange_.get().y}}};
    const mat3 basis(corners[1] - corners[0], corners[2] - corners[0], corners[3] - corners[0]);

    // data range
    float maxVelocity{0};
    float maxCurl{0};
    const float maxDiv{std::abs(-1 - sigma_.get() - beta_.get())};

    for (auto x : {xRange_.get().x, xRange_.get().y}) {
        for (auto y : {yRange_.get().x, yRange_.get().y}) {
            for (auto z : {zRange_.get().x, zRange_.get().y}) {
                maxVelocity = std::max({maxVelocity, std::abs(sigma_.get() * (y - x)),
                                        std::abs(x * (rho_.get() - z) - y),
                                        std::abs(x * y - beta_.get() * z)});
                maxCurl = std::max({maxCurl, std::abs(2 * x), std::abs(-y),
                                    std::abs(rho_.get() - z - sigma_.get())});
            }
        }
    }

    velocityVolume->setBasis(basis);
    velocityVolume->setOffset(corners[0]);
    velocityVolume->dataMap.dataRange = dvec2{-maxVelocity, maxVelocity};
    velocityVolume->dataMap.valueRange = dvec2{-maxVelocity, maxVelocity};

    curlVolume->setBasis(basis);
    curlVolume->setOffset(corners[0]);
    curlVolume->dataMap.dataRange = dvec2{-maxCurl, maxCurl};
    curlVolume->dataMap.valueRange = dvec2{-maxCurl, maxCurl};

    divVolume->setBasis(basis);
    divVolume->setOffset(corners[0]);
    divVolume->dataMap.dataRange = dvec2{-maxDiv, maxDiv};
    divVolume->dataMap.valueRange = dvec2{-maxDiv, maxDiv};

    shader_.activate();
    utilgl::setShaderUniforms(shader_, *velocityVolume, "volumeParameters");
    utilgl::setShaderUniforms(shader_, rho_);
    utilgl::setShaderUniforms(shader_, sigma_);
    utilgl::setShaderUniforms(shader_, beta_);

    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dims.x), static_cast<GLsizei>(dims.y));

    std::array<GLenum, 3> drawBuffers{0};

    auto* volGL = velocityVolume->getEditableRepresentation<VolumeGL>();
    drawBuffers[0] = fbo_.attachColorTexture(volGL->getTexture().get(), 0);

    auto* curlGL = curlVolume->getEditableRepresentation<VolumeGL>();
    drawBuffers[1] = fbo_.attachColorTexture(curlGL->getTexture().get(), 1);

    auto* divGL = divVolume->getEditableRepresentation<VolumeGL>();
    drawBuffers[2] = fbo_.attachColorTexture(divGL->getTexture().get(), 2);

    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dims.z));

    shader_.deactivate();
    fbo_.deactivate();

    velocityOutport_.setData(velocityVolume);
    curlOutport_.setData(curlVolume);
    divOutport_.setData(divVolume);
}

}  // namespace inviwo
