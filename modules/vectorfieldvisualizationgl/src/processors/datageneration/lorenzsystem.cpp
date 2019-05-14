/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LorenzSystem::processorInfo_{
    "org.inviwo.LorenzSystem",  // Class identifier
    "Lorenz System",            // Display name
    "Data Creation",            // Category
    CodeState::Stable,          // Code state
    "GL, Generator",            // Tags
};
const ProcessorInfo LorenzSystem::getProcessorInfo() const { return processorInfo_; }

LorenzSystem::LorenzSystem()
    : Processor()
    , outport_("outport")
    , curlOutport_("curl")
    , divOutport_("divergence")

    , size_("size", "Volume size", size3_t(32, 32, 32), size3_t(1, 1, 1), size3_t(1024, 1024, 1024))
    , xRange_("xRange", "X Range", -20, 20, -100, 100)
    , yRange_("yRange", "Y Range", -30, 30, -100, 100)
    , zRange_("zRange", "Z Range", 0, 50, 0, 100)
    , rhoValue_("rho", "&rho; Value", 28, 0, 100)
    , sigmaValue_("sigma", "&sigma; Value", 10, 0, 100)
    , betaValue_("beta", "&beta; Value", 8.0f / 3.0f, 0, 100)
    , shader_("volume_gpu.vert", "volume_gpu.geom", "lorenzsystem.frag")
    , fbo_() {
    addPort(outport_);
    addPort(curlOutport_);
    addPort(divOutport_);

    addProperty(size_);

    addProperty(xRange_);
    addProperty(yRange_);
    addProperty(zRange_);

    addProperty(rhoValue_);
    addProperty(sigmaValue_);
    addProperty(betaValue_);

    shader_.onReload([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

LorenzSystem::~LorenzSystem() {}

void LorenzSystem::process() {
    auto volume = std::make_shared<Volume>(size_.get(), DataVec3Float32::get());
    volume->dataMap_.dataRange = vec2(0, 1);
    volume->dataMap_.valueRange = vec2(-1, 1);

    auto curlvolume = std::make_shared<Volume>(size_.get(), DataVec3Float32::get());
    curlvolume->dataMap_.dataRange = vec2(0, 1);
    curlvolume->dataMap_.valueRange = vec2(-1, 1);

    auto divvolume = std::make_shared<Volume>(size_.get(), DataFloat32::get());
    divvolume->dataMap_.dataRange = vec2(0, 1);
    divvolume->dataMap_.valueRange = vec2(-1, 1);

    // Basis and offset
    vec3 corners[4];
    corners[0] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().x);
    corners[1] = vec3(xRange_.get().y, yRange_.get().x, zRange_.get().x);
    corners[2] = vec3(xRange_.get().x, yRange_.get().y, zRange_.get().x);
    corners[3] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().y);

    mat3 basis(corners[1] - corners[0], corners[2] - corners[0], corners[3] - corners[0]);

    volume->setBasis(basis);
    volume->setOffset(corners[0]);
    curlvolume->setBasis(basis);
    curlvolume->setOffset(corners[0]);
    divvolume->setBasis(basis);
    divvolume->setOffset(corners[0]);

    outport_.setData(volume);
    curlOutport_.setData(curlvolume);
    divOutport_.setData(divvolume);

    shader_.activate();
    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *outport_.getData(), "volume");

    utilgl::setShaderUniforms(shader_, rhoValue_);
    utilgl::setShaderUniforms(shader_, sigmaValue_);
    utilgl::setShaderUniforms(shader_, betaValue_);
    const size3_t dim{size_.get()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    VolumeGL* volGL = volume->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(volGL->getTexture().get(), 0);

    VolumeGL* curlGL = curlvolume->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(curlGL->getTexture().get(), 1);

    VolumeGL* divGL = divvolume->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(divGL->getTexture().get(), 2);

    fbo_.defineDrawBuffers();

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();
}

}  // namespace inviwo
