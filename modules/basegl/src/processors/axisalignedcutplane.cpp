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

#include <modules/basegl/processors/axisalignedcutplane.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AxisAlignedCutPlane::processorInfo_{
    "org.inviwo.AxisAlignedCutPlane",  // Class identifier
    "Axis Aligned Cut Plane",          // Display name
    "Volume Operation",                // Category
    CodeState::Experimental,           // Code state
    Tags::GL,                          // Tags
};
const ProcessorInfo AxisAlignedCutPlane::getProcessorInfo() const { return processorInfo_; }

AxisAlignedCutPlane::AxisAlignedCutPlane()
    : Processor()
    , volume_("volume")
    , imageInport_("imageInport_")
    , outport_("outport")
    , xSlide_("x", "X Slide")
    , ySlide_("y", "Y Slide")
    , zSlide_("z", "Z Slide")
    , channel_("channel", "Channel", {{"channel0", "Channel 1", 0}})
    , disableTF_("disableTF", "Disable transfer function", false,
                 InvalidationLevel::InvalidResources)
    , tf_("transferfunction", "Transfer function", &volume_)
    , showBoundingBox_("boundingBox", "Show Bounding Box", true)
    , boundingBoxColor_("boundingBoxColor", "Bounding Box Color", vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , renderPointSize_("renderPointSize", "Point Size", 1.0f, 0.001f, 15.0f, 0.001f)
    , renderLineWidth_("renderLineWidth", "Line Width", 1.0f, 0.001f, 15.0f, 0.001f)
    , nearestInterpolation_("nearestInterpolation", "Use nearest neighbor interpolation", false)
    , camera_("camera", "Camera", util::boundingBox(volume_))
    , trackball_(&camera_)
    , sliceShader_("geometryrendering.vert", "axisalignedcutplaneslice.frag", false)
    , boundingBoxShader_("geometryrendering.vert", "axisalignedcutplaneboundingbox.frag") {

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
    addProperty(renderPointSize_);
    addProperty(renderLineWidth_);

    addProperty(camera_);
    addProperty(trackball_);

    imageInport_.setOptional(true);

    tf_.get().clear();
    tf_.get().add(0.0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
    tf_.get().add(1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f));

    tf_.setCurrentStateAsDefault();

    sliceShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    boundingBoxShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    xSlide_.onChange([&]() {
        if (volume_.hasData()) xSlide_.createDrawer(volume_.getData());
    });
    ySlide_.onChange([&]() {
        if (volume_.hasData()) ySlide_.createDrawer(volume_.getData());
    });
    zSlide_.onChange([&]() {
        if (volume_.hasData()) zSlide_.createDrawer(volume_.getData());
    });

    volume_.onChange([&]() {
        if (!volume_.hasData()) return;
        auto vol = volume_.getData();
        xSlide_.onVolumeChange(vol);
        ySlide_.onVolumeChange(vol);
        zSlide_.onVolumeChange(vol);
        if (!boundingBoxMesh_) {
            createBoundingBox();
        }
        boundingBoxMesh_->setModelMatrix(vol->getModelMatrix());
        boundingBoxMesh_->setWorldMatrix(vol->getWorldMatrix());

        // Update channel option property
        if (channel_.size() != vol->getDataFormat()->getComponents()) {
            auto curC = channel_.getSelectedIndex();

            for (auto i = channel_.size(); i < vol->getDataFormat()->getComponents(); i++) {
                channel_.addOption("channel" + std::to_string(i),
                                   "Channel " + std::to_string(i + 1), static_cast<int>(i));
            }

            while (channel_.size() > vol->getDataFormat()->getComponents()) {
                channel_.removeOption(channel_.size() - 1);
            }

            channel_.setSelectedIndex(0);
            channel_.setCurrentStateAsDefault();

            channel_.setSelectedIndex(std::min(curC, channel_.size() - 1));
        }
    });

    boundingBoxColor_.setSemantics(PropertySemantics::Color);

    setAllPropertiesCurrentStateAsDefault();

    createBoundingBox();
}

void AxisAlignedCutPlane::process() {
    if (imageInport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    if (!boundingBoxDrawer_) {
        boundingBoxDrawer_ =
            getNetwork()->getApplication()->getMeshDrawerFactory()->create(boundingBoxMesh_.get());
    }

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);

    drawBoundingBox();
    TextureUnitContainer cont;

    sliceShader_.activate();

    utilgl::setUniforms(sliceShader_, camera_);
    utilgl::bindAndSetUniforms(sliceShader_, cont, volume_);

    if (nearestInterpolation_.get()) {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
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
    boundingBoxMesh_ = std::make_unique<SimpleMesh>(DrawType::Lines, ConnectivityType::Strip);
    boundingBoxMesh_->addVertex(vec3(0, 0, 0), vec3(0, 0, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(0, 0, 1), vec3(0, 0, 1), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(0, 1, 0), vec3(0, 1, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(0, 1, 1), vec3(0, 1, 1), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 0, 0), vec3(1, 0, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 0, 1), vec3(1, 0, 1), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 1, 0), vec3(1, 1, 0), boundingBoxColor_.get());
    boundingBoxMesh_->addVertex(vec3(1, 1, 1), vec3(1, 1, 1), boundingBoxColor_.get());

    boundingBoxMesh_->addIndices(1, 0, 2, 3, 7, 5, 4, 6, 2, 0, 4, 5, 1, 3, 7, 6);
}

void AxisAlignedCutPlane::drawBoundingBox() {
    if (showBoundingBox_.get() == false) return;
    boundingBoxShader_.activate();
    utilgl::setShaderUniforms(boundingBoxShader_, *boundingBoxMesh_, "geometry");
    utilgl::setUniforms(boundingBoxShader_, camera_, boundingBoxColor_);

    utilgl::PolygonModeState polygon(GL_LINE, renderLineWidth_, renderPointSize_);
    boundingBoxDrawer_->draw();
    boundingBoxShader_.deactivate();
}

}  // namespace inviwo
