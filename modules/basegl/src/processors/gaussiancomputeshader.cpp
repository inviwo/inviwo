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

#include <modules/basegl/processors/gaussiancomputeshader.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/base/algorithm/dataminmax.h>         // for dataMinMax
#include <inviwo/core/util/glmcomp.h>            // for compMul

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GaussianComputeShader::processorInfo_{
    "org.inviwo.GaussianComputeShader",  // Class identifier
    "Gaussian Compute Shader",           // Display name
    "Experimental",                      // Category
    CodeState::Experimental,             // Code state
    Tags::GL | Tag("Compute Shader"),    // Tags
    R"(Creating Gaussian Volume using Compute Shader)"_unindentHelp};

const ProcessorInfo GaussianComputeShader::getProcessorInfo() const { return processorInfo_; }

GaussianComputeShader::GaussianComputeShader()
    : Processor{}  //, volume_{"volume"}
                   //, positionlayer_{"positions"}
    , points_{"points", "Imported points"_help}
    , outport_{"outport"}
    , shaderGaussian_{{{ShaderType::Compute, "gaussian.comp"}}}
    , dimensions_("dimensions", "Dimensions", size3_t(64), size3_t(1), size3_t(512))
    , groupSize_("groupsize", "GroupSize", size3_t(16), size3_t(1), size3_t(32))
    , sigma_("sigma", "Sigma", 1.0f, 0.0f, 10.0f) {

    addProperties(dimensions_, groupSize_, sigma_);
    addPorts(points_, outport_);

    shaderGaussian_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void GaussianComputeShader::initializeResources() {
    // auto* compShader = shaderGaussian_.getComputeShaderObject();

    // compShader->addShaderDefine("MY_DEFINE", toString(2));
    shaderGaussian_.build();
}

void GaussianComputeShader::process() {
    TextureUnit unit;
    unit.activate();

    auto vol = std::make_shared<Volume>(dimensions_, DataFormat<float>::get());

    auto volGL = vol->getEditableRepresentation<VolumeGL>();
    auto texHandle = volGL->getTexture()->getID();
    glBindImageTexture(unit.getUnitNumber(), texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    // layerGL->setSwizzleMask(swizzlemasks::luminance);

    shaderGaussian_.activate();
    utilgl::setUniforms(shaderGaussian_, sigma_);

    volGL->getTexture()->bind();

    shaderGaussian_.setUniform("dest", unit.getUnitNumber());
    auto ptr = points_.getData();
    auto points = *ptr;
    struct alignas(16) CustomType {
        vec4 p;
        vec4 p2;
    };
    std::vector<CustomType> customPoints(points.size());
    std::transform(std::begin(points), std::end(points), std::begin(customPoints),
                   [](const vec4& p) {
                          
                       CustomType newPoint{p, vec4{1, 2, 3, 16}};
                       return newPoint;
                   });

    size3_t dims = dimensions_;
    size3_t groupSize = groupSize_;
    shaderGaussian_.setUniform("dims", ivec3(dims));
    shaderGaussian_.setUniform("groupSize", ivec3(groupSize));

    GLuint buffHandle;
    glGenBuffers(1, &buffHandle);
    size_t bufferSize{points.size() * sizeof(CustomType)};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, customPoints.data(), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffHandle, 0, bufferSize);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    uvec3 numWorkGroups{(uvec3(dims) + uvec3(groupSize) - uvec3(1)) /
                        uvec3(groupSize)};  // Divide by work group size (e.g., 4)
    numWorkGroups.z = 1;
    shaderGaussian_.setUniform("workGroup", numWorkGroups);
    for (int z = 0; z < dims.z; ++z) {
        // Set the zOffset uniform to focus on the current z-slice
        shaderGaussian_.setUniform("zOffset", z);

        // Dispatch compute shader to process one slice (1 workgroup per z-slice)

        glDispatchCompute(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);

        // Wait for the GPU to finish processing the current slice
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    shaderGaussian_.deactivate();

    // debugging of texture contents
    // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    std::vector<float> tmp;
    tmp.resize(glm::compMul(vol->getDimensions()));
    volGL->getTexture()->download(tmp.data());
    // +++++++++++++++++++++++++++++++++++

    auto* data = static_cast<const VolumeRAMPrecision<float>*>(vol->getRepresentation<VolumeRAM>())->getDataTyped();
    auto minmax = util::dataMinMax(data, glm::compMul(dims), IgnoreSpecialValues::Yes);
    
    //auto minmax = util::dataMinMax(data, glm::compMul(dimensions), IgnoreSpecialValues::Yes);
    mat3 basis(1);
    vol->dataMap.dataRange = dvec2 { 0.0, minmax.second.x };
    vol->dataMap.valueRange = dvec2{0.0, 40.0};
    vol->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    outport_.setData(vol);
}

}  // namespace inviwo
