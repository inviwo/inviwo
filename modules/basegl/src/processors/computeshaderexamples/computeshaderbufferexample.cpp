/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/basegl/processors/computeshaderexamples/computeshaderbufferexample.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opengl/buffer/buffergl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ComputeShaderBufferExample::processorInfo_{
    "org.inviwo.ComputeShaderBufferExample",  // Class identifier
    "Compute Shader Buffer Example",          // Display name
    "Example",                                // Category
    CodeState::Experimental,                  // Code state
    Tags::None,                               // Tags
};
const ProcessorInfo ComputeShaderBufferExample::getProcessorInfo() const { return processorInfo_; }

ComputeShaderBufferExample::ComputeShaderBufferExample()
    : Processor()
    , mesh_("mesh")
    , shader_({{ShaderType::Compute, "spiral.comp"}})
    , numPoints_("numPoints", "Number of points", 1024, 1, 10000)
    , radius_("radius", "Radius", 1, 0, 10)
    , rotations_("rotations", "Rotations", 1, 0, 10)
    , height_("height", "Height", 1, 0, 10)
    , tf_("tf", "Color Mapping") {
    addPort(mesh_);

    addProperties(numPoints_, radius_, rotations_, height_, tf_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void ComputeShaderBufferExample::process() {
    shader_.activate();

    shader_.setUniform("numPoints", numPoints_.get());
    shader_.setUniform("radius", radius_.get());
    shader_.setUniform("rotations", rotations_.get());
    shader_.setUniform("height", height_.get());

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, tf_);

    auto bufPos = std::make_shared<Buffer<vec4>>(numPoints_.get(), BufferUsage::Dynamic);
    auto bufCol = std::make_shared<Buffer<vec4>>(numPoints_.get(), BufferUsage::Dynamic);
    auto bugPosGL = bufPos->getEditableRepresentation<BufferGL>();
    auto bugColGL = bufCol->getEditableRepresentation<BufferGL>();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bugPosGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bugColGL->getId());

    glDispatchCompute(numPoints_.get(), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    shader_.deactivate();

    auto mesh = std::make_shared<Mesh>();
    mesh->addBuffer(BufferType::PositionAttrib, bufPos);
    mesh->addBuffer(BufferType::ColorAttrib, bufCol);

    auto ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip);
    auto &vec = ib->getDataContainer();
    vec.resize(numPoints_.get());
    std::iota(vec.begin(), vec.end(), 0);

    mesh_.setData(mesh);
}

}  // namespace inviwo
