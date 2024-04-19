/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/pathtracing/processors/uniformgridopacitygl.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo UniformGridOpacityGL::processorInfo_{
    "org.inviwo.uniformgridopacitygl",  // Class identifier
    "Uniform Grid Opacity GL",          // Display name
    "Volume",                           // Category
    CodeState::Experimental,            // Code state
    Tags::GL,                           // Tags
};

const ProcessorInfo UniformGridOpacityGL::getProcessorInfo() const { return processorInfo_; }

UniformGridOpacityGL::UniformGridOpacityGL()
    : Processor{}
    , inport_{"data"}
    , outportVolume_{"MinMaxOpacityVolume"}
    , transferFunction_("transferFunction", "Transfer function")
    , volumeRegionSize_("region", "Region size", 8, 1, 100)
    , shader_({{ShaderType::Compute, "minmaxopacity/minmaxavg.comp"}}) {

    addPort(inport_);
    // addPort(outport_);
    addPort(outportVolume_);

    addProperty(transferFunction_);
    addProperty(volumeRegionSize_);

    shader_.onReload([this]() {
        shader_.build();
        invalidate(InvalidationLevel::InvalidOutput);
        
    });
}

void UniformGridOpacityGL::process() {
    auto curVolume = inport_.getData();

    auto dim = curVolume->getDimensions();
    auto region = inviwo::ivec3(volumeRegionSize_.get());
    const size3_t outDim{glm::ceil(vec3(dim) / static_cast<float>(volumeRegionSize_.get()))};


    auto statsRAMRep = std::make_shared<VolumeRAMPrecision<vec4>>(
        outDim, swizzlemasks::rgba, InterpolationType::Nearest, curVolume->getWrapping());

    // GL rep construction. TODO: Do we need to add a dataformat id (see formats.h /.cpp)?
    auto statsGLRep =
        std::make_shared<VolumeGL>(outDim, DataVec4Float32::get(), swizzlemasks::rgba,
                                   InterpolationType::Nearest, curVolume->getWrapping());

    auto stats = std::make_shared<Volume>(statsGLRep);

    stats->dataMap.dataRange.x = 0;  // should map from 0,1
    stats->dataMap.dataRange.y = 1;  // should map from 0,1
    stats->dataMap.valueRange.x = 0;
    stats->dataMap.valueRange.y = 1;

    stats->setModelMatrix(curVolume->getModelMatrix());
    stats->setWorldMatrix(curVolume->getWorldMatrix());
    stats->setDimensions(outDim);

    // Compute Shader set up
    shader_.activate();

    TextureUnitContainer units;

    shader_.setUniform("regionSize", region);

    utilgl::bindAndSetUniforms(shader_, units, *inport_.getData(), "volumeData");
    // I need to write to it, and i dont think bindandset lets me do that

    /*utilgl::bindAndSetUniforms(shader_, units, *stats, "opacityData"); // with write capabilites*/
    {
        TextureUnit unit;
        auto statsGL = stats->getEditableRepresentation<VolumeGL>();
        statsGL->bindImageTexture(unit.getEnum(), unit.getUnitNumber(), GL_WRITE_ONLY);

        shader_.setUniform("opacityData", unit);

        units.push_back(std::move(unit));

        StrBuffer buff;

        utilgl::setShaderUniforms(shader_, *stats,
                                  buff.replace("{}Parameters", outportVolume_.getIdentifier()));
    }

    utilgl::bindAndSetUniforms(shader_, units, transferFunction_);

    // dispatch size? The workload can be anything from 1 to 150^3
    // 150/regionsize in all dimensions? We can start with a 'naive' (stupid) implementation and let
    // the dispatch be for stats.dim. Each compute shader will do the inner 3loops work.

    glDispatchCompute(outDim.x, outDim.y, outDim.z);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    shader_.deactivate();

    outportVolume_.setData(stats);
}

}  // namespace inviwo
