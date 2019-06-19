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

#include <modules/vectorfieldvisualizationgl/processors/3d/streamparticles.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opengl/buffer/buffergl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

#include <inviwo/core/util/volumesequenceutils.h>
#include <inviwo/core/util/zip.h>

#include <random>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamParticles::processorInfo_{
    "org.inviwo.StreamParticles",  // Class identifier
    "Stream Particles",            // Display name
    "Particle",                    // Category
    CodeState::Experimental,       // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo StreamParticles::getProcessorInfo() const { return processorInfo_; }

StreamParticles::StreamParticles(InviwoApplication *app) : Processor() {

    addPort(volume_);
    addPort(seeds_);
    addPort(meshPort_);

    addProperties(seedingSpace_, stepLength_, internalSteps_, particleSize_, minV_, maxV_, tf_,
                  reseedInterval_);

    tf_.get().load(app->getPath(PathType::TransferFunctions, "/matplotlib/plasma.itf"));
    tf_.setCurrentStateAsDefault();

    shader_.onReload([this]() {
        invalidate(InvalidationLevel::InvalidOutput);
        buffersDirty = true;
    });

    seedingSpace_.onChange([=]() { buffersDirty = true; });
    seeds_.onChange([=]() { buffersDirty = true; });
    // volume_.onChange([this]() { buffersDirty = true; });

    timer_.start();
}

void StreamParticles::process() {
    initBuffers();
    reseed();

    advect();

    meshPort_.setData(mesh_);
}

void StreamParticles::update() {
    if (mutex_.try_lock()) {
        dispatchFront([this]() {
            invalidate(InvalidationLevel::InvalidOutput);
            mutex_.unlock();
        });
    }
}

void StreamParticles::initBuffers() {
    if (!buffersDirty) return;
    buffersDirty = false;

    auto seeds = seeds_.getData();

    auto numPoints = seeds->size();

    bufPos_ = std::make_shared<Buffer<vec4>>(numPoints, BufferUsage::Dynamic);
    bufLife_ = std::make_shared<Buffer<float>>(numPoints, BufferUsage::Dynamic);
    bufRad_ = std::make_shared<Buffer<float>>(numPoints, BufferUsage::Dynamic);
    bufCol_ = std::make_shared<Buffer<vec4>>(numPoints, BufferUsage::Dynamic);

    std::mt19937 rand;
    std::uniform_real_distribution<float> dist(0, 1);

    auto &positions = bufPos_->getEditableRAMRepresentation()->getDataContainer();
    auto &lifes = bufLife_->getEditableRAMRepresentation()->getDataContainer();

    if (seedingSpace_.get() == SeedingSpace::World) {
        std::transform(seeds->begin(), seeds->end(), positions.begin(),
                       [](vec3 seed) { return vec4(seed, 1.0f); });
    } else {
        std::transform(
            seeds->begin(), seeds->end(), positions.begin(),
            [toWorld =
                 volume_.getData()->getCoordinateTransformer().getDataToWorldMatrix()](vec3 seed) {
                vec4 p = toWorld * vec4(seed, 1.0);
                return p / p.w;
            });
    }

    std::fill(lifes.begin(), lifes.end(), 1.0f);

    bufPos_->getEditableRepresentation<BufferGL>();
    bufLife_->getEditableRepresentation<BufferGL>();
    bufRad_->getEditableRepresentation<BufferGL>();
    bufCol_->getEditableRepresentation<BufferGL>();

    mesh_ = std::make_shared<Mesh>();
    mesh_->addBuffer(BufferType::PositionAttrib, bufPos_);
    mesh_->addBuffer(BufferType::RadiiAttrib, bufRad_);
    mesh_->addBuffer(BufferType::ColorAttrib, bufCol_);

    reseedtime = c.getElapsedSeconds();
}
void StreamParticles::advect() {
    TextureUnitContainer cont;

    auto volume = volume_.getData();

    auto bufPosGL = bufPos_->getEditableRepresentation<BufferGL>();
    auto bufLifeGL = bufLife_->getEditableRepresentation<BufferGL>();
    auto bufRadGL = bufRad_->getEditableRepresentation<BufferGL>();
    auto bufColGL = bufCol_->getEditableRepresentation<BufferGL>();

    shader_.activate();

    utilgl::setUniforms(shader_, stepLength_, internalSteps_, minV_, maxV_);
    utilgl::bindAndSetUniforms(shader_, cont, tf_);

    auto toWorld = volume->getCoordinateTransformer().getTextureToWorldMatrix();
    auto toTexture = volume->getCoordinateTransformer().getWorldToTextureMatrix();

    shader_.setUniform("minR", particleSize_.get().x);
    shader_.setUniform("maxR", particleSize_.get().y);
    shader_.setUniform("toWorldMatrix", toWorld);
    shader_.setUniform("toTextureMatrix", toTexture);

    utilgl::bindAndSetUniforms(shader_, cont, *volume, "velocityField");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPosGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufLifeGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufRadGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufColGL->getId());

    glDispatchCompute(static_cast<GLuint>(seeds_.getData()->size()), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    shader_.deactivate();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
}
void StreamParticles::reseed() {
    auto curT = c.getElapsedSeconds();
    if (curT >= reseedtime + reseedInterval_.get()) {
        size_t count = 0;
        auto seeds = seeds_.getData();

        auto &positions = bufPos_->getEditableRAMRepresentation()->getDataContainer();
        auto &lifes = bufLife_->getEditableRAMRepresentation()->getDataContainer();

        std::mt19937 rand;
        std::uniform_int_distribution<size_t> dist(0, seeds->size());

        auto seedTransform =
            [space = seedingSpace_.get(),
             toWorld = volume_.getData()->getCoordinateTransformer().getDataToWorldMatrix()](
                vec3 seed) -> vec4 {
            if (space == SeedingSpace::World) {
                return vec4(seed, 1.0f);
            } else {
                vec4 p = toWorld * vec4(seed, 1.0);
                return p / p.w;
            }
        };

        for (auto z : util::zip(positions, lifes)) {
            if (get<1>(z) <= 0) {
                get<0>(z) = seedTransform((*seeds)[dist(rand)]);
                get<1>(z) = 1.0f;
                count++;
            }
        }

        LogInfo("Reseeded " << count << " particles");
        reseedtime = curT;
    }
}  // namespace inviwo

}  // namespace inviwo
