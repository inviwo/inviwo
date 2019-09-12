/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/basegl/processors/meshpicking.h>

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

const ProcessorInfo MeshPicking::processorInfo_{
    "org.inviwo.GeometryPicking",  // Class identifier
    "Mesh Picking",                // Display name
    "Mesh Rendering",              // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo MeshPicking::getProcessorInfo() const { return processorInfo_; }

MeshPicking::MeshPicking()
    : Processor()
    , meshInport_("geometryInport")
    , imageInport_("imageInport")
    , outport_("outport")
    , compositor_()
    , cullFace_("cullFace", "Cull Face",
                {{"culldisable", "Disable", GL_NONE},
                 {"cullfront", "Front", GL_FRONT},
                 {"cullback", "Back", GL_BACK},
                 {"cullfrontback", "Front & Back", GL_FRONT_AND_BACK}},
                2)
    , position_("position", "Position", vec3(0.0f), vec3(-100.f), vec3(100.f))
    , highlightColor_("highlightColor", "Highlight Color", vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , camera_("camera", "Camera", util::boundingBox(meshInport_))
    , trackball_(&camera_)
    , picking_(this, 1, [&](PickingEvent* p) { handlePickingEvent(p); })
    , shader_("standard.vert", "picking.frag") {

    imageInport_.setOptional(true);

    addPort(meshInport_);
    addPort(imageInport_);
    addPort(outport_);

    addProperty(cullFace_);
    addProperty(position_);

    highlightColor_.setSemantics(PropertySemantics::Color);
    addProperty(highlightColor_);

    addProperty(camera_);
    addProperty(trackball_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

MeshPicking::~MeshPicking() = default;

void MeshPicking::handlePickingEvent(PickingEvent* p) {
    if (p->getState() == PickingState::Updated && p->getEvent()->hash() == MouseEvent::chash()) {
        auto me = p->getEventAs<MouseEvent>();
        if ((me->buttonState() & MouseButton::Left) && me->state() == MouseState::Move) {
            updatePosition(p);
        }
    } else if (p->getState() == PickingState::Updated &&
               p->getEvent()->hash() == TouchEvent::chash()) {

        auto te = p->getEventAs<TouchEvent>();
        if (!te->touchPoints().empty() && te->touchPoints()[0].state() == TouchState::Updated) {
            updatePosition(p);
        }
    } else if (auto we = p->getEventAs<WheelEvent>()) {
        p->markAsUsed();

        double Zn = camera_.getNearPlaneDist();
        double Zf = camera_.getFarPlaneDist();

        dvec3 camDir(glm::normalize(camera_.get().getDirection()));

        position_.set(position_.get() + vec3(0.01 * (Zf - Zn) * we->delta().y * camDir));
    }

    if (p->getState() == PickingState::Started) {
        highlight_ = true;
        invalidate(InvalidationLevel::InvalidOutput);
    } else if (p->getState() == PickingState::Finished) {
        highlight_ = false;
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void MeshPicking::updatePosition(PickingEvent* p) {
    p->markAsUsed();

    auto currNDC = p->getNDC();
    auto prevNDC = p->getPreviousNDC();

    // Use depth of initial press as reference to move in the image plane.
    auto refDepth = p->getPressedDepth();
    currNDC.z = refDepth;
    prevNDC.z = refDepth;

    auto corrWorld = camera_.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(currNDC));
    auto prevWorld = camera_.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(prevNDC));

    position_.set(position_.get() + (corrWorld - prevWorld));
}

void MeshPicking::process() {
    if (meshInport_.isChanged()) {
        mesh_ = meshInport_.getData();
        drawer_ = std::make_unique<MeshDrawerGL>(mesh_.get());
    }

    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

    shader_.activate();
    shader_.setUniform("pickingColor", picking_.getColor());
    shader_.setUniform("highlight", highlight_);
    utilgl::setShaderUniforms(shader_, highlightColor_);

    const auto& ct = mesh_->getCoordinateTransformer(camera_.get());
    const auto dataToClip =
        ct.getWorldToClipMatrix() * glm::translate(position_.get()) * ct.getDataToWorldMatrix();
    shader_.setUniform("dataToClip", dataToClip);

    {
        utilgl::CullFaceState culling(cullFace_.get());
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::DepthFuncState depthfunc(GL_ALWAYS);

        drawer_->draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    if (imageInport_.hasData()) {
        compositor_.composite(imageInport_, outport_, ImageType::ColorDepthPicking);
    }
}

}  // namespace inviwo
