/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "entryexitpoints.h"
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/rendering/geometryrendererfactory.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/clockgl.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(EntryExitPoints, "org.inviwo.EntryExitPoints");
ProcessorDisplayName(EntryExitPoints, "Entry Exit Points");
ProcessorTags(EntryExitPoints, Tags::GL);
ProcessorCategory(EntryExitPoints, "Geometry Rendering");
ProcessorCodeState(EntryExitPoints, CODE_STATE_STABLE);

EntryExitPoints::EntryExitPoints()
    : Processor()
    , geometryPort_("geometry")
    , entryPort_("entry-points", DataVec4UINT16::get())
    , exitPort_("exit-points", DataVec4UINT16::get())
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), &geometryPort_)
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
    , trackball_(&camera_)
    , genericShader_(NULL)
    , capNearClippingPrg_(NULL)
    , tmpEntryPoints_(NULL)
    , renderer_(NULL) {
    addPort(geometryPort_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(capNearClipping_);
    addProperty(camera_);
    addProperty(trackball_);
    entryPort_.addResizeEventListener(&camera_);
    geometryPort_.onChange(this, &EntryExitPoints::onGeometryChange);
}

EntryExitPoints::~EntryExitPoints() {}

void EntryExitPoints::initialize() {
    Processor::initialize();
    genericShader_ = new Shader("standard.vert", "standard.frag");
    capNearClippingPrg_ = new Shader("img_identity.vert", "capnearclipping.frag");
}

void EntryExitPoints::deinitialize() {
    delete tmpEntryPoints_;
    tmpEntryPoints_ = NULL;
    delete genericShader_;
    genericShader_ = NULL;
    delete capNearClippingPrg_;
    capNearClippingPrg_ = NULL;
    delete renderer_;
    renderer_ = NULL;
    Processor::deinitialize();
}

void EntryExitPoints::process() {
    const Geometry* geom = geometryPort_.getData();

    // Check if no renderer exist or if geometry changed
    if (renderer_ == NULL) {
        onGeometryChange();
    }

    if (renderer_ == NULL) {
        return;
    }

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_ALWAYS);
    // generate exit points
    utilgl::activateAndClearTarget(exitPort_, COLOR_DEPTH);
    glPointSize(1.f);
    glCullFace(GL_FRONT);
    genericShader_->activate();

    mat4 modelMatrix = geom->getCoordinateTransformer(&camera_).getDataToClipMatrix();
    genericShader_->setUniform("dataToClip_", modelMatrix);

    renderer_->render();
    utilgl::deactivateCurrentTarget();
    // generate entry points
    ImageGL* tmpEntryPointsGL = NULL;

    if (capNearClipping_.get()) {
        if (tmpEntryPoints_ == NULL ||
            tmpEntryPoints_->getDimensions() != entryPort_.getDimensions() ||
            tmpEntryPoints_->getDataFormat() != entryPort_.getData()->getDataFormat()) {
            delete tmpEntryPoints_;
            tmpEntryPoints_ =
                new Image(entryPort_.getDimensions(), entryPort_.getData()->getDataFormat());
        }
        tmpEntryPointsGL = tmpEntryPoints_->getEditableRepresentation<ImageGL>();
        tmpEntryPointsGL->activateBuffer();
        utilgl::clearCurrentTarget();

    } else {
        utilgl::activateAndClearTarget(entryPort_, COLOR_DEPTH);
    }

    glCullFace(GL_BACK);
    renderer_->render();
    genericShader_->deactivate();
    utilgl::deactivateCurrentTarget();

    if (capNearClipping_.get() && tmpEntryPointsGL) {
        // render an image plane aligned quad to cap the proxy geometry
        utilgl::activateAndClearTarget(entryPort_, COLOR_DEPTH);
        TextureUnit entryColorUnit, entryDepthUnit, exitColorUnit, exitDepthUnit;
        tmpEntryPointsGL->getColorLayerGL()->bindTexture(entryColorUnit.getEnum());
        tmpEntryPointsGL->getDepthLayerGL()->bindTexture(entryDepthUnit.getEnum());
        utilgl::bindTextures(exitPort_, exitColorUnit.getEnum(), exitDepthUnit.getEnum());
        capNearClippingPrg_->activate();

        capNearClippingPrg_->setUniform("entryColorTex_", entryColorUnit.getUnitNumber());
        capNearClippingPrg_->setUniform("entryDepthTex_", entryDepthUnit.getUnitNumber());
        utilgl::setShaderUniforms(capNearClippingPrg_, entryPort_, "entryParameters_");
        capNearClippingPrg_->setUniform("exitColorTex_", exitColorUnit.getUnitNumber());
        capNearClippingPrg_->setUniform("exitDepthTex_", exitDepthUnit.getUnitNumber());
        utilgl::setShaderUniforms(capNearClippingPrg_, exitPort_, "exitParameters_");
        // the rendered plane is specified in camera coordinates
        // thus we must transform from camera to world to texture coordinates
        mat4 clipToTexMat = geom->getCoordinateTransformer(&camera_).getClipToDataMatrix();       
        capNearClippingPrg_->setUniform("NDCToTextureMat_", clipToTexMat);
        
        capNearClippingPrg_->setUniform("nearDist_", camera_.getNearPlaneDist());
        utilgl::singleDrawImagePlaneRect();
        capNearClippingPrg_->deactivate();
        utilgl::deactivateCurrentTarget();
    }

    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
}

void EntryExitPoints::onGeometryChange() {
    delete renderer_;
    renderer_ = NULL;
    if (geometryPort_.hasData())
        renderer_ = GeometryRendererFactory::getPtr()->create(geometryPort_.getData());
}

}  // namespace
