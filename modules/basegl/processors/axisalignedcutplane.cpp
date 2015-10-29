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

#include "axisalignedcutplane.h"

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AxisAlignedCutPlane::processorInfo_{
    "org.inviwo.AxisAlignedCutPlane",  // Class identifier
    "Axis Aligned Cut Plane",          // Display name
    "Volume Operation",                // Category
    CodeState::Experimental,           // Code state
    Tags::GL,                          // Tags
};
const ProcessorInfo AxisAlignedCutPlane::getProcessorInfo() const {
    return processorInfo_;
}

AxisAlignedCutPlane::AxisAlignedCutPlane()
    : Processor()
    , volume_("volume")
    , imageInport_("imageInport_")
    , outport_("outport")
    , xSlide_("x", "X Slide")
    , ySlide_("y", "Y Slide")
    , zSlide_("z", "Z Slide")
    , disableTF_("disableTF", "Disable transfer function", false, InvalidationLevel::InvalidResources)
    , tf_("transferfunction", "Transfer function" , TransferFunction(), &volume_)
    , sliceShader_("geometryrendering.vert", "axisalignedcutplaneslice.frag", false)
    , boundingBoxShader_("geometryrendering.vert", "axisalignedcutplaneboundingbox.frag")
    , showBoundingBox_("boundingBox", "Show Bounding Box", true)
    , boundingBoxColor_("boundingBoxColor", "Bounding Box Color", vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , nearestInterpolation_("nearestInterpolation","Use nearest neighbor interpolation" , false)
    , camera_("camera", "Camera")
    , trackball_(&camera_) {
    addPort(volume_);
    addPort(imageInport_);
    addPort(outport_);

    addProperty(xSlide_);
    addProperty(ySlide_);
    addProperty(zSlide_);
    addProperty(disableTF_);
    addProperty(tf_);
    addProperty(showBoundingBox_);
    addProperty(boundingBoxColor_);

    addProperty(camera_);
    addProperty(trackball_);

    imageInport_.setOptional(true);



    tf_.get().clearPoints();
    tf_.get().addPoint(vec2(0.0f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));
    tf_.get().addPoint(vec2(1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));

    tf_.setCurrentStateAsDefault();

    
    xSlide_.onChange([&]() { if (volume_.hasData()) xSlide_.createDrawer(volume_.getData()); });
    ySlide_.onChange([&]() { if (volume_.hasData()) ySlide_.createDrawer(volume_.getData()); });
    zSlide_.onChange([&]() { if (volume_.hasData()) zSlide_.createDrawer(volume_.getData()); });

    volume_.onChange([&]() {
        if (!volume_.hasData()) return;
        auto vol = volume_.getData();
        xSlide_.onVolumeChange(vol);
        ySlide_.onVolumeChange(vol);
        zSlide_.onVolumeChange(vol);
        boundingBoxMesh_->setModelMatrix(vol->getModelMatrix());
        boundingBoxMesh_->setWorldMatrix(vol->getWorldMatrix());
    });

    boundingBoxColor_.setSemantics(PropertySemantics::Color);

    setAllPropertiesCurrentStateAsDefault();

    createBoundingBox();
}

void AxisAlignedCutPlane::process() {
    if (imageInport_.isConnected()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_ , ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);

    drawBoundingBox();
    TextureUnitContainer cont;

    sliceShader_.activate();
    utilgl::setShaderUniforms(sliceShader_, camera_, "camera_");
    utilgl::bindAndSetUniforms(sliceShader_, cont, volume_);
    if (nearestInterpolation_.get()) {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    if (!disableTF_.get()) {
        utilgl::bindAndSetUniforms(sliceShader_, cont, tf_);
    }
    xSlide_.draw(sliceShader_);
    ySlide_.draw(sliceShader_);
    zSlide_.draw(sliceShader_);
    sliceShader_.deactivate();

    utilgl::deactivateCurrentTarget();
}

void AxisAlignedCutPlane::createBoundingBox() {
    boundingBoxMesh_ = util::make_unique<SimpleMesh>(DrawType::LINES, ConnectivityType::STRIP);
    boundingBoxMesh_->addVertex(vec3(0, 0, 0), vec3(0, 0, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(0, 0, 1), vec3(0, 0, 1), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(0, 1, 0), vec3(0, 1, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(0, 1, 1), vec3(0, 1, 1), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 0, 0), vec3(1, 0, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 0, 1), vec3(1, 0, 1), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 1, 0), vec3(1, 1, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 1, 1), vec3(1, 1, 1), boundingBoxColor_.get());

    boundingBoxMesh_->addIndices(1, 0, 2, 3, 7, 5, 4, 6, 2, 0, 4, 5, 1, 3, 7, 6);

    boundingBoxDrawer_ = MeshDrawerFactory::getPtr()->create(boundingBoxMesh_.get());
}

void AxisAlignedCutPlane::drawBoundingBox() {
    if (showBoundingBox_.get() == false) return;
    boundingBoxShader_.activate();
    utilgl::setShaderUniforms(boundingBoxShader_, camera_, "camera_");
    utilgl::setShaderUniforms(boundingBoxShader_, *boundingBoxMesh_, "geometry_");
    utilgl::setShaderUniforms(boundingBoxShader_, boundingBoxColor_);
    boundingBoxDrawer_->draw();
    boundingBoxShader_.deactivate();
}

}  // namespace

