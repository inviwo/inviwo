/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator4d.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VectorFieldGenerator4D::processorInfo_{
    "org.inviwo.VectorFieldGenerator4D",  // Class identifier
    "Vector Field Generator 4D",          // Display name
    "Data Creation",                      // Category
    CodeState::Stable,                    // Code state
    "GL, Generator",                      // Tags
};
const ProcessorInfo VectorFieldGenerator4D::getProcessorInfo() const { return processorInfo_; }

VectorFieldGenerator4D::VectorFieldGenerator4D()
    : Processor()
    , outport_("outport")

    , size_("size", "Volume size", size4_t(16), size4_t(1), size4_t(1024))
    , xValue_("x", "X", "-y", InvalidationLevel::InvalidResources)
    , yValue_("y", "Y", "x", InvalidationLevel::InvalidResources)
    , zValue_("z", "Z", "(1-sqrt(x*x+y*y))*0.4", InvalidationLevel::InvalidResources)
    , xRange_("xRange", "X Range", -1, 1, -10, 10)
    , yRange_("yRange", "Y Range", -1, 1, -10, 10)
    , zRange_("zRange", "Z Range", -1, 1, -10, 10)
    , tRange_("tRange", "T Range", 0, 1, -10, 10)
    , shader_("volume_gpu.vert", "volume_gpu.geom", "vectorfieldgenerator4d.frag", false)
    , fbo_() {
    addPort(outport_);

    addProperty(size_);
    addProperty(xValue_);
    addProperty(yValue_);
    addProperty(zValue_);

    addProperty(xRange_);
    addProperty(yRange_);
    addProperty(zRange_);
    addProperty(tRange_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::Valid); });
}

void VectorFieldGenerator4D::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("X_VALUE(x,y,z,t)", xValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("Y_VALUE(x,y,z,t)", yValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("Z_VALUE(x,y,z,t)", zValue_.get());

    shader_.build();
}

void VectorFieldGenerator4D::process() {

    auto seq = std::make_shared<VolumeSequenceOutport::type>();

    float dt = (tRange_.get().y - tRange_.get().x) / (size_.get().w - 1);

    vec3 corners[4];
    corners[0] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().x);
    corners[1] = vec3(xRange_.get().y, yRange_.get().x, zRange_.get().x);
    corners[2] = vec3(xRange_.get().x, yRange_.get().y, zRange_.get().x);
    corners[3] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().y);

    mat3 basis;
    basis[0] = corners[1] - corners[0];
    basis[1] = corners[2] - corners[0];
    basis[2] = corners[3] - corners[0];

    shader_.activate();
    fbo_.activate();
    const size3_t dim{size_.get()};
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));
    for (size_t i = 0; i < size_.get().w; ++i) {
        float t = static_cast<float>(i);
        t /= size_.get().w - 1;
        t = tRange_.get().x + t * (tRange_.get().y - tRange_.get().x);

        auto volume = std::make_shared<Volume>(size3_t(size_.get()), DataVec3Float32::get());
        volume->dataMap_.dataRange = vec2(0, 1);
        volume->dataMap_.valueRange = vec2(-1, 1);
        volume->setBasis(basis);
        volume->setOffset(corners[0]);
        volume->setMetaData<DoubleMetaData>("timestamp", t);
        volume->setMetaData<DoubleMetaData>("duration", dt);

        TextureUnitContainer cont;
        utilgl::bindAndSetUniforms(shader_, cont, *volume.get(), "volume");

        shader_.setUniform("t", t);

        VolumeGL* outVolumeGL = volume->getEditableRepresentation<VolumeGL>();
        fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);

        utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

        seq->push_back(volume);
    }
    shader_.deactivate();
    fbo_.deactivate();

    outport_.setData(seq);
}

}  // namespace inviwo
