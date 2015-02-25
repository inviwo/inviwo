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

#include "geometrypicking.h"

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <modules/opengl/rendering/meshdrawer.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(GeometryPicking, "org.inviwo.GeometryPicking");
ProcessorDisplayName(GeometryPicking, "Geometry Picking");
ProcessorTags(GeometryPicking, Tags::GL);
ProcessorCategory(GeometryPicking, "Geometry Rendering");
ProcessorCodeState(GeometryPicking, CODE_STATE_STABLE);

GeometryPicking::GeometryPicking()
    : CompositeProcessorGL()
    , geometryInport_("geometryInport")
    , imageInport_("imageInport")
    , outport_("outport")
    , position_("position", "Position", vec3(0.0f), vec3(-100.f), vec3(100.f))
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f))
    , trackball_(&camera_) {
    addPort(geometryInport_);
    addPort(imageInport_);
    addPort(outport_);
    addProperty(position_);
    addProperty(camera_);
    addProperty(trackball_);
}

GeometryPicking::~GeometryPicking() {}

void GeometryPicking::initialize() {
    CompositeProcessorGL::initialize();
    shader_ = new Shader("standard.vert", "picking.frag");
    widgetPickingObject_ = PickingManager::getPtr()->registerPickingCallback(
        this, &GeometryPicking::updateWidgetPositionFromPicking);
}

void GeometryPicking::deinitialize() {
    CompositeProcessorGL::deinitialize();
    delete shader_;
    shader_ = nullptr;
    PickingManager::getPtr()->unregisterPickingObject(widgetPickingObject_);
}

void GeometryPicking::updateWidgetPositionFromPicking(const PickingObject* p) {
    vec2 move = p->getPickingMove();

    if (move.x == 0.f && move.y == 0.f) return;

    vec2 pos = p->getPickingPosition();
    float depth = static_cast<float>(p->getPickingDepth());
    vec3 startNdc = vec3(2.f * pos - 1.f, 2.f * depth - 1.f);
    vec3 endNdc = vec3(2.f * (pos + move) - 1.f, 2.f * depth - 1.f);
    vec3 startWorld = camera_.getWorldPosFromNormalizedDeviceCoords(startNdc);
    vec3 endWorld = camera_.getWorldPosFromNormalizedDeviceCoords(endNdc);
    position_.set(position_.get() + (endWorld - startWorld));
    invalidate(INVALID_OUTPUT);
}

void GeometryPicking::process() {
    utilgl::activateAndClearTarget(outport_, COLOR_DEPTH_PICKING);
    MeshDrawer drawer(static_cast<const Mesh*>(geometryInport_.getData()));
    shader_->activate();
    shader_->setUniform("pickingColor_", widgetPickingObject_->getPickingColor());

    const SpatialCameraCoordinateTransformer<3>& ct =
        geometryInport_.getData()->getCoordinateTransformer(&camera_);

    mat4 dataToClip_ =
        ct.getWorldToClipMatrix() * glm::translate(position_.get()) * ct.getDataToWorldMatrix();

    shader_->setUniform("dataToClip_", dataToClip_);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_ALWAYS);
    drawer.draw();
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);

    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
    compositePortsToOutport(outport_, COLOR_DEPTH_PICKING, imageInport_);
}

}  // namespace
