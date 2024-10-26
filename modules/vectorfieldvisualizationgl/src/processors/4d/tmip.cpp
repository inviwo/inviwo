/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/4d/tmip.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, DataInport
#include <inviwo/core/ports/datainport.h>                               // for DataInport
#include <inviwo/core/ports/volumeport.h>                               // for VolumeOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/util/glmvec.h>                                    // for size3_t, uvec3
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>                                // for glGetIntegerv
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                           // for ShaderType, Shade...
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for multiDrawImagePla...
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms

#include <algorithm>      // for min
#include <cmath>          // for ceil
#include <sstream>        // for operator<<, basic...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <glm/vec3.hpp>  // for vec<>::(anonymous)

namespace inviwo {
class DataFormatBase;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TMIP::processorInfo_{
    "org.inviwo.TMIP",            // Class identifier
    "TMIP",                       // Display name
    "Volume Sequence Operation",  // Category
    CodeState::Experimental,      // Code state
    Tags::GL,                     // Tags
};

const ProcessorInfo& TMIP::getProcessorInfo() const { return processorInfo_; }

TMIP::TMIP()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , outputType_("outputType", "Output type",
                  {{"scalar", "Maximum Scalar", OutputType::Scalar},
                   {"velocity", "Vector with maximum velocity", OutputType::HighestVelocity}})
    , shader_("volume_gpu.vert", "volume_gpu.geom", "tmip.frag", Shader::Build::No)
    , shaderLast_("volume_gpu.vert", "volume_gpu.geom", "tmip.frag", Shader::Build::No)
    , fbo_() {

    addPort(inport_);
    addPort(outport_);

    addProperty(outputType_);

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
        volume0_->dataMap = firstVol->dataMap;

        volume1_ = std::shared_ptr<Volume>(volume0_->clone());
    }

    int iterations =
        static_cast<int>(std::ceil(volumes->size() / static_cast<float>(maxSamplers_)));

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
        } else if (!lastIT) {
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

        switch (outputType_.get()) {
            case OutputType::Scalar:
                if (i == 0) {
                    maximum << "result = sample0;";
                } else {
                    maximum << "result = max(result,sample" << i << ");";
                }
                break;
            case OutputType::HighestVelocity:
                if (i == 0) {
                    maximum << "result = sample0;result.a = length(sample0.xyz);";
                } else {
                    maximum << "{ float l = length(sample" << i
                            << ".xyz); if(l > result.a)   result = vec4(sample" << i << ".xyz,l);}";
                }
                break;
        }
    }

    s[ShaderType::Fragment]->addShaderDefine("GEN_UNIFORMS", uniforms.str());
    s[ShaderType::Fragment]->addShaderDefine("GEN_SAMPLING", sampling.str());
    s[ShaderType::Fragment]->addShaderDefine("GEN_MAX", maximum.str());
    s.build();
}

std::shared_ptr<Volume> TMIP::iteration(Shader& s, std::shared_ptr<Volume> vol,
                                        std::shared_ptr<Volume> target,
                                        std::vector<std::shared_ptr<Volume>>::const_iterator start,
                                        std::vector<std::shared_ptr<Volume>>::const_iterator end) {
    s.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(s, cont, *vol, "volume");
    int i = 1;
    for (auto v = start; v != end; ++v) {
        if (i >= maxSamplers_) break;
        std::string id = (i == 0) ? "" : toString(i);
        utilgl::bindAndSetUniforms(s, cont, *(v->get()), "volume" + id);
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

    s.deactivate();
    fbo_.deactivate();

    return vol;
}

}  // namespace inviwo
