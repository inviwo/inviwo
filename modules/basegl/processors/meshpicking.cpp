/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "meshpicking.h"

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

ProcessorClassIdentifier(MeshPicking, "org.inviwo.GeometryPicking");
ProcessorDisplayName(MeshPicking, "Mesh Picking");
ProcessorTags(MeshPicking, Tags::GL);
ProcessorCategory(MeshPicking, "Geometry Rendering");
ProcessorCodeState(MeshPicking, CODE_STATE_STABLE);

MeshPicking::MeshPicking()
    : CompositeProcessorGL()
    , meshInport_("geometryInport")
    , imageInport_("imageInport")
    , outport_("outport")
    , cullFace_("cullFace", "Cull Face")
    , position_("position", "Position", vec3(0.0f), vec3(-100.f), vec3(100.f))
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f))
    , trackball_(&camera_)
    , shader_("standard.vert", "picking.frag") {

    addPort(meshInport_);
    addPort(imageInport_);
    addPort(outport_);
    outport_.addResizeEventListener(&camera_);

    cullFace_.addOption("culldisable", "Disable", GL_NONE);
    cullFace_.addOption("cullfront", "Front", GL_FRONT);
    cullFace_.addOption("cullback", "Back", GL_BACK);
    cullFace_.addOption("cullfrontback", "Front & Back", GL_FRONT_AND_BACK);
    cullFace_.set(GL_BACK);
    addProperty(cullFace_);

    addProperty(position_);
    addProperty(camera_);
    addProperty(trackball_);

    shader_.onReload([this]() { invalidate(INVALID_RESOURCES); });

    widgetPickingObject_ = PickingManager::getPtr()->registerPickingCallback(
        this, &MeshPicking::updateWidgetPositionFromPicking);
}

MeshPicking::~MeshPicking() {
    PickingManager::getPtr()->unregisterPickingObject(widgetPickingObject_);
}

void MeshPicking::updateWidgetPositionFromPicking(const PickingObject* p) {
    vec2 move = p->getPickingMove();

    if (move.x == 0.f && move.y == 0.f) return;

    vec2 pos = p->getPickingPosition();
    float depth = static_cast<float>(p->getPickingDepth());
    vec3 startNdc = vec3(2.f * pos - 1.f, depth);
    vec3 endNdc = vec3(2.f * (pos + move) - 1.f, depth);
    vec3 startWorld = camera_.getWorldPosFromNormalizedDeviceCoords(startNdc);
    vec3 endWorld = camera_.getWorldPosFromNormalizedDeviceCoords(endNdc);
    position_.set(position_.get() + (endWorld - startWorld));
    invalidate(INVALID_OUTPUT);
}

void MeshPicking::process() {
    utilgl::activateAndClearTarget(outport_, COLOR_DEPTH_PICKING);

    MeshDrawerGL drawer(meshInport_.getData().get());
    shader_.activate();
    shader_.setUniform("pickingColor_", widgetPickingObject_->getPickingColor());

    const auto& ct = meshInport_.getData()->getCoordinateTransformer(camera_.get());

    mat4 dataToClip =
        ct.getWorldToClipMatrix() * glm::translate(position_.get()) * ct.getDataToWorldMatrix();

    shader_.setUniform("dataToClip", dataToClip);

    {
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::CullFaceState culling(cullFace_.get());
        utilgl::DepthFuncState depthfunc(GL_ALWAYS);
        drawer.draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    compositePortsToOutport(outport_, COLOR_DEPTH_PICKING, imageInport_);
}

}  // namespace
