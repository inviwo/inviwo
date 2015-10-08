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

#include "vectorfieldgenerator3d.h"
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

    ProcessorClassIdentifier(VectorFieldGenerator3D, "org.inviwo.VectorFieldGenerator3D");
    ProcessorDisplayName(VectorFieldGenerator3D, "Vector Field Generator 3D");
    ProcessorTags(VectorFieldGenerator3D, "GL");
    ProcessorCategory(VectorFieldGenerator3D, "Data Creation");
    ProcessorCodeState(VectorFieldGenerator3D, CODE_STATE_EXPERIMENTAL);

    VectorFieldGenerator3D::VectorFieldGenerator3D()
        : Processor()
        , outport_("outport")
        , shader_("volume_gpu.vert", "volume_gpu.geom", "vectorfieldgenerator3d.frag", false)
        , fbo_()
        , size_("size", "Volume size", size3_t(16), size3_t(1), size3_t(1024))
        , xValue_("x", "X", "-y", INVALID_RESOURCES)
        , yValue_("y", "Y", "x", INVALID_RESOURCES)
        , zValue_("z", "Z", "(1-sqrt(x*x+y*y))*0.4", INVALID_RESOURCES)
        , xRange_("xRange", "X Range", -1, 1, -10, 10)
        , yRange_("yRange", "Y Range", -1, 1, -10, 10)
        , zRange_("zRange", "Z Range", -1, 1, -10, 10)
    {
        addPort(outport_);

        addProperty(size_);
        addProperty(xValue_);
        addProperty(yValue_);
        addProperty(zValue_);

        addProperty(xRange_);
        addProperty(yRange_);
        addProperty(zRange_);
    }

VectorFieldGenerator3D::~VectorFieldGenerator3D()  {
    
}


void VectorFieldGenerator3D::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("X_VALUE(x,y,z)", xValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("Y_VALUE(x,y,z)", yValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("Z_VALUE(x,y,z)", zValue_.get());

    shader_.build();
}

void VectorFieldGenerator3D::process() {

    volume_ = std::make_shared<Volume>(size_.get(), DataVec4FLOAT32::get());
    volume_->dataMap_.dataRange = vec2(0, 1);
    volume_->dataMap_.valueRange = vec2(-1, 1);
    outport_.setData(volume_);

    shader_.activate();
    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *volume_.get(), "volume");
    utilgl::setUniforms(shader_, xRange_, yRange_, zRange_);
    const size3_t dim{ size_.get() };
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    VolumeGL* outVolumeGL = volume_->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();

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

}

} // namespace

