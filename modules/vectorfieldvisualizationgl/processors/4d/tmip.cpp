/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "tmip.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/util/shuntingyard.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

    // The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
    const ProcessorInfo TMIP::processorInfo_{
        "org.inviwo.TMIP",  // Class identifier
        "TMIP",           // Display name
        "Volume Vector Operation",                  // Category
        CodeState::Experimental,          // Code state
        Tags::GL,                        // Tags
    };

    const ProcessorInfo TMIP::getProcessorInfo() const {
        return processorInfo_;
    }

TMIP::TMIP()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , outputType_("outputType","Output type")
    , shader_("volume_gpu.vert", "volume_gpu.geom", "tmip.frag", false)
    , shaderLast_("volume_gpu.vert", "volume_gpu.geom", "tmip.frag", false)
    , fbo_() {
    addPort(inport_);
    addPort(outport_);

    addProperty(outputType_);
    outputType_.addOption("scalar", "Maximum Scalar", OutputType::Scalar);
    outputType_.addOption("velocity", "Vector with maximum velocity", OutputType::HighestVelocity);

    inport_.onChange([this]() { initializeResources(); });

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxSamplers_);
    maxSamplers_ -= 1;
}

void TMIP::process() {
    auto volumes = inport_.getData();

    if (volumes->empty()) {
        return;
    }

    auto firstVol = volumes->at(0);

    if (inport_.isChanged()) {
        const DataFormatBase* format = firstVol->getDataFormat();
        volume0_ = std::make_shared<Volume>(firstVol->getDimensions(), format);
        volume0_->setModelMatrix(firstVol->getModelMatrix());
        volume0_->setWorldMatrix(firstVol->getWorldMatrix());
        // pass on metadata
        volume0_->copyMetaDataFrom(*firstVol);
        volume0_->dataMap_ = firstVol->dataMap_;

        volume1_ = std::shared_ptr<Volume>(volume0_->clone());
    }

    int iterations = static_cast<int>(std::ceil(volumes->size() / static_cast<float>(maxSamplers_)));

    std::shared_ptr<Volume> readVol = volume0_;
    std::shared_ptr<Volume> writeVol = volume1_;
    int offset = 0;
    for (int i = 0; i < iterations; i++) {
        bool firstIT = i == 0;
        bool lastIT = i != 0 && iterations;
        
        auto startVolIT = volumes->begin() + offset + 1;
        
        if (firstIT) {
            auto endVolIT = volumes->begin() + offset + maxSamplers_;
            iteration(shader_, volumes->at(0), writeVol, startVolIT, endVolIT);
        }
        else if (!lastIT) {
            auto endVolIT = volumes->begin() + offset + maxSamplers_;
            iteration(shader_, readVol, writeVol, startVolIT, endVolIT);
        } else {
            iteration(shaderLast_, readVol, writeVol, startVolIT, volumes->end());
        }
        std::swap(readVol, writeVol);
        offset += maxSamplers_;
    }

    outport_.setData(readVol);
}

void TMIP::initializeResources() {
    auto volumes = inport_.getData();

    int numVolumes = static_cast<int>(volumes->size());
    auto samplers = std::min(maxSamplers_, numVolumes);

    initShader(shader_, samplers);

    if (numVolumes > maxSamplers_) {
        samplers = (numVolumes - maxSamplers_) % (maxSamplers_ - 1) + 1;
        initShader(shaderLast_, samplers);
    }
}

void TMIP::initShader(Shader& s, int samplers) {
    std::stringstream uniforms;
    std::stringstream sampling;
    std::stringstream maximum;

    uniforms << "uniform VolumeParameters volumeParameters;";

    for (int i = 0; i < samplers; i++) {
        std::string id = (i == 0) ? "" : toString(i);
        uniforms << "uniform sampler3D volume" << id << ";";
        sampling << "vec4 sample" << i << " = getVoxel(volume" << id
                 << ", volumeParameters, texCoord_.xyz);";

        switch (outputType_.get())
        {
        case OutputType::Scalar:
            if (i == 0) {
                maximum << "result = sample0;";
            }
            else {
                maximum << "result = max(result,sample" << i << ");";
            }
            break;
        case OutputType::HighestVelocity:
            if (i == 0) {
                maximum << "result = sample0.xyz;result.a = length(sample0.xyz);";
            }
            else {
                maximum << "{ float l = length(sample" << i
                    << ".xyz); if(l > result.a)   result = vec4(sample" << i << ".xyz,l);}";
            }
            break;
        }

        i++;
    }

    shader_.getFragmentShaderObject()->addShaderDefine("GEN_UNIFORMS", uniforms.str());
    shader_.getFragmentShaderObject()->addShaderDefine("GEN_SAMPLING", sampling.str());
    shader_.getFragmentShaderObject()->addShaderDefine("GEN_MAX", maximum.str());
    shader_.build();
}

std::shared_ptr<Volume> TMIP::iteration(Shader& s, std::shared_ptr<Volume> vol, std::shared_ptr<Volume> target,
                                        std::vector<std::shared_ptr<Volume>>::const_iterator start,
                                        std::vector<std::shared_ptr<Volume>>::const_iterator end) {
    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *vol, "volume");
    int i = 1;
    for (auto v = start; v != end; ++v) {
        if (i >= maxSamplers_) break;
        std::string id = (i == 0) ? "" : toString(i);
        utilgl::bindAndSetUniforms(shader_, cont, *(v->get()), "volume" + id);
        i++;
    }

    const size3_t dim{vol->getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    VolumeGL* outVolumeGL = volume0_->getEditableRepresentation<VolumeGL>();
    if (inport_.isChanged()) {
        fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);
    }

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();

    return vol;
}

}  // namespace
