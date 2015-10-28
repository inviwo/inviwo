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

#include "lorenzsystem.h"
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
    CodeState::Experimental,    // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo LorenzSystem::getProcessorInfo() const {
    return processorInfo_;
}

LorenzSystem::LorenzSystem()
    : Processor()
    , outport_("outport")
    , shader_("volume_gpu.vert", "volume_gpu.geom", "lorenzsystem.frag")
    , fbo_()
    , size_("size", "Volume size", size3_t(32, 32, 32), size3_t(1, 1, 1), size3_t(1024, 1024, 1024))
    , xRange_("xRange", "X Range", -20, 20, -100, 100)
    , yRange_("yRange", "Y Range", -30, 30, -100, 100)
    , zRange_("zRange", "Z Range", 0, 50, 0, 100)
    , rhoValue_("rho", "&rho; Value", 28, 0, 100)
    , sigmaValue_("sigma", "&sigma; Value", 10, 0, 100)
    , betaValue_("beta", "&beta; Value", 8.0f / 3, 0, 100) {
    addPort(outport_);

    addProperty(size_);

    addProperty(xRange_);
    addProperty(yRange_);
    addProperty(zRange_);

    addProperty(rhoValue_);
    addProperty(sigmaValue_);
    addProperty(betaValue_);
}

LorenzSystem::~LorenzSystem() {}

void LorenzSystem::process() {
    volume_ = std::make_shared<Volume>(size_.get(), DataVec4Float32::get());
    volume_->dataMap_.dataRange = vec2(0, 1);
    volume_->dataMap_.valueRange = vec2(-1, 1);
    outport_.setData(volume_);

    shader_.activate();
    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *outport_.getData(), "volume");
    shader_.setUniformWarningLevel(Shader::UniformWarning::Warn);

    utilgl::setShaderUniforms(shader_, xRange_);
    utilgl::setShaderUniforms(shader_, yRange_);
    utilgl::setShaderUniforms(shader_, zRange_);
    utilgl::setShaderUniforms(shader_, rhoValue_);
    utilgl::setShaderUniforms(shader_, sigmaValue_);
    utilgl::setShaderUniforms(shader_, betaValue_);
    const size3_t dim{size_.get()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    VolumeGL* outVolumeGL = volume_->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    vec3 corners[5];
    corners[0] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().x);
    corners[1] = vec3(xRange_.get().y, yRange_.get().x, zRange_.get().x);
    corners[2] = vec3(xRange_.get().x, yRange_.get().y, zRange_.get().x);
    corners[3] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().y);
    corners[4] = vec3(xRange_.get().y, yRange_.get().y, zRange_.get().y);

    mat3 basis;
    vec3 basisX = corners[1] - corners[0];
    vec3 basisY = corners[2] - corners[0];
    vec3 basisZ = corners[3] - corners[0];

    basis[0][0] = basisX.x;
    basis[0][1] = basisX.y;
    basis[0][2] = basisX.z;
    basis[1][0] = basisY.x;
    basis[1][1] = basisY.y;
    basis[1][2] = basisY.z;
    basis[2][0] = basisZ.x;
    basis[2][1] = basisZ.y;
    basis[2][2] = basisZ.z;
    volume_->setBasis(basis);
    volume_->setOffset(corners[0]);

    shader_.deactivate();
    fbo_.deactivate();
}

}  // namespace

