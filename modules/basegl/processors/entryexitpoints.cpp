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
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/clockgl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

ProcessorClassIdentifier(EntryExitPoints, "org.inviwo.EntryExitPoints");
ProcessorDisplayName(EntryExitPoints, "Entry Exit Points");
ProcessorTags(EntryExitPoints, Tags::GL);
ProcessorCategory(EntryExitPoints, "Geometry Rendering");
ProcessorCodeState(EntryExitPoints, CODE_STATE_STABLE);

EntryExitPoints::EntryExitPoints()
    : Processor()
    , inport_("geometry")
    , entryPort_("entry", DataVec4UINT16::get())
    , exitPort_("exit", DataVec4UINT16::get())
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), &inport_)
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
    , trackball_(&camera_)
    , shader_("standard.vert", "standard.frag")
    , clipping_("img_identity.vert", "capnearclipping.frag")
    , tmpEntry_()
    , drawer_() {
    addPort(inport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(capNearClipping_);
    addProperty(camera_);
    addProperty(trackball_);
    entryPort_.addResizeEventListener(&camera_);

    shader_.onReload([this]() { invalidate(INVALID_RESOURCES); });
    clipping_.onReload([this]() { invalidate(INVALID_RESOURCES); });
}

EntryExitPoints::~EntryExitPoints() {}

void EntryExitPoints::process() {
    // Check if no renderer exist or if geometry changed
    if (inport_.isChanged() && inport_.hasData()) {
        drawer_ = MeshDrawerFactory::getPtr()->create(inport_.getData());
    }
    if (!drawer_) return;

    utilgl::DepthFuncState depthfunc(GL_ALWAYS);
    utilgl::PointSizeState pointsize(1.0f);

    shader_.activate();
    const Mesh* geom = inport_.getData();
    mat4 modelMatrix = geom->getCoordinateTransformer(camera_.get()).getDataToClipMatrix();
    shader_.setUniform("dataToClip", modelMatrix);

    {
        // generate exit points
        utilgl::activateAndClearTarget(exitPort_, COLOR_DEPTH);
        utilgl::CullFaceState cull(GL_FRONT);
        drawer_->draw();
        utilgl::deactivateCurrentTarget();
    }

    {
        // generate entry points
        if (capNearClipping_) {
            if (!tmpEntry_ ||
                tmpEntry_->getDimensions() != entryPort_.getDimensions() ||
                tmpEntry_->getDataFormat() != entryPort_.getData()->getDataFormat()) {
                tmpEntry_.reset(
                    new Image(entryPort_.getDimensions(), entryPort_.getData()->getDataFormat()));
            }
            utilgl::activateAndClearTarget(tmpEntry_.get());
        } else {
            utilgl::activateAndClearTarget(entryPort_, COLOR_DEPTH);
        }

        utilgl::CullFaceState cull(GL_BACK);
        drawer_->draw();
        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }

    if (capNearClipping_ && tmpEntry_) {
        // render an image plane aligned quad to cap the proxy geometry
        utilgl::activateAndClearTarget(entryPort_, COLOR_DEPTH);
        clipping_.activate();

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(&clipping_, units, tmpEntry_.get(), "entry", COLOR_DEPTH);
        utilgl::bindAndSetUniforms(&clipping_, units, exitPort_, COLOR_DEPTH);

        // the rendered plane is specified in camera coordinates
        // thus we must transform from camera to world to texture coordinates
        mat4 clipToTexMat = geom->getCoordinateTransformer(camera_.get()).getClipToDataMatrix();
        clipping_.setUniform("NDCToTextureMat", clipToTexMat);
        clipping_.setUniform("nearDist", camera_.getNearPlaneDist());

        utilgl::singleDrawImagePlaneRect();
        clipping_.deactivate();
        utilgl::deactivateCurrentTarget();
    }
}

void EntryExitPoints::deserialize(IvwDeserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace
