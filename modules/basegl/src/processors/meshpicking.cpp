/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>                 // for boundingBox
#include <inviwo/core/datastructures/camera/camera.h>          // for Camera
#include <inviwo/core/datastructures/coordinatetransformer.h>  // for SpatialCameraCoordinateTra...
#include <inviwo/core/datastructures/image/image.h>            // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>       // for ImageType, ImageType::Colo...
#include <inviwo/core/interaction/cameratrackball.h>           // for CameraTrackball
#include <inviwo/core/interaction/events/interactionevent.h>   // for InteractionEvent
#include <inviwo/core/interaction/events/mousebuttons.h>       // for MouseButton, MouseButton::...
#include <inviwo/core/interaction/events/mouseevent.h>         // for MouseEvent
#include <inviwo/core/interaction/events/pickingevent.h>       // for PickingEvent
#include <inviwo/core/interaction/events/touchevent.h>         // for TouchEvent, TouchPoint
#include <inviwo/core/interaction/events/touchstate.h>         // for TouchState, TouchState::Up...
#include <inviwo/core/interaction/events/wheelevent.h>         // for WheelEvent
#include <inviwo/core/interaction/pickingmapper.h>             // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>              // for PickingState, PickingState...
#include <inviwo/core/ports/imageport.h>                       // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>                        // for MeshInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::GL
#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>          // for InvalidationLevel, Invalid...
#include <inviwo/core/properties/optionproperty.h>             // for OptionPropertyOption, Opti...
#include <inviwo/core/properties/ordinalproperty.h>            // for FloatVec3Property, FloatVe...
#include <inviwo/core/properties/propertysemantics.h>          // for PropertySemantics, Propert...
#include <inviwo/core/util/glmvec.h>                           // for vec3, dvec2, dvec3, size2_t
#include <modules/opengl/image/imagecompositor.h>              // for ImageCompositor
#include <modules/opengl/inviwoopengl.h>                       // for GL_BACK, GL_DEPTH_TEST
#include <modules/opengl/openglutils.h>                        // for CullFaceState, DepthFuncState
#include <modules/opengl/rendering/meshdrawergl.h>             // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                      // for Shader
#include <modules/opengl/shader/shaderutils.h>                 // for setShaderUniforms
#include <modules/opengl/texture/textureutils.h>               // for activateAndClearTarget

#include <functional>   // for __base, function
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_reference<>::type
#include <vector>       // for vector

#include <flags/flags.h>                 // for operator&, flags
#include <glm/ext/matrix_transform.hpp>  // for translate
#include <glm/geometric.hpp>             // for normalize
#include <glm/gtx/transform.hpp>         // for translate
#include <glm/mat4x4.hpp>                // for operator*, mat
#include <glm/vec2.hpp>                  // for operator!=, vec<>::(anonym...
#include <glm/vec3.hpp>                  // for operator*, operator+, vec
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {

const ProcessorInfo MeshPicking::processorInfo_{
    "org.inviwo.GeometryPicking",  // Class identifier
    "Mesh Picking",                // Display name
    "Mesh Rendering",              // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
    R"(Renders a Mesh with an optional background image. The mesh can be repositioned through picking.
    Use the left mouse button to move the mesh around.)"_unindentHelp,
};
const ProcessorInfo& MeshPicking::getProcessorInfo() const { return processorInfo_; }

MeshPicking::MeshPicking()
    : Processor()
    , meshInport_("geometryInport")
    , imageInport_("imageInport")
    , outport_("outport")
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

    if (imageInport_.isReady()) {
        if (!tmp_ || tmp_->getDimensions() != outport_.getDimensions() ||
            tmp_->getDataFormat() != outport_.getDataFormat()) {
            tmp_.emplace(outport_.getDimensions(), outport_.getDataFormat());
        }
        if (!compositor_) {
            compositor_.emplace();
        }
        utilgl::activateAndClearTarget(*tmp_, ImageType::ColorDepthPicking);
        render();
        compositor_->composite(*imageInport_.getData(), *tmp_, *outport_.getEditableData(),
                               ImageType::ColorDepthPicking);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);
        render();
    }
}

void MeshPicking::render() {
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
        utilgl::DepthFuncState depthfunc(GL_LESS);

        drawer_->draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
