/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/processors/gizmorenderer.h>

#include <inviwo/core/interaction/events/event.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GizmoRenderer::processorInfo_{
    "org.inviwo.GizmoRenderer",     // Class identifier
    "Gizmo Renderer",               // Display name
    "UI",                           // Category
    CodeState::Stable,              // Code state
    Tags::GL | Tag{"Interaction"},  // Tags
    R"(Renders interactive transform gizmos on top of an optional background image and propagates
       events first to the gizmo.)"_unindentHelp,
};

const ProcessorInfo& GizmoRenderer::getProcessorInfo() const { return processorInfo_; }

GizmoRenderer::GizmoRenderer()
    : Processor{}
    , inport_{"inport", "Gizmo"_help}
    , imageInport_{"imageInport", "Background image (optional)"_help}
    , outport_{"outport",
               "Output image with the interaction handles rendered on top of the "
               "background image"_help}
    , gizmoProperties_{"gizmo", "Gizmo Settings"}
    , camera_{"camera", "Camera"}
    , trackball_{&camera_} {

    imageInport_.setOptional(true);
    addPorts(inport_, imageInport_, outport_);
    addProperties(gizmoProperties_, camera_, trackball_);
}

void GizmoRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    glClear(gl::ClearBufferMask::GL_DEPTH_BUFFER_BIT);
    const utilgl::BlendModeState blend{gl::GLenum::GL_SRC_ALPHA,
                                       gl::GLenum::GL_ONE_MINUS_SRC_ALPHA};

    const auto& gizmoData = *inport_.getData();

    if (gizmoData.type == GizmoType::None) {
        utilgl::deactivateCurrentTarget();
        return;
    }

    GizmoConfig config;
    gizmoProperties_.update(config);

    // render gizmo
    gizmoutil::renderGizmo(outport_, camera_.get(), gizmoData, config);

    utilgl::deactivateCurrentTarget();
}

void GizmoRenderer::invokeEvent(Event* e) { Processor::invokeEvent(e); }

void GizmoRenderer::propagateEvent(Event* event, Outport* source) {
    if (!event->markAsVisited(this)) return;

    bool used = false;
    // propagate events first to gizmo, for example a picking event
    if (event->shouldPropagateTo(&inport_, this, source)) {
        inport_.propagateEvent(event);
        used |= event->markAsUnused();
    }
    event->setUsed(used);

    invokeEvent(event);
    used = event->hasBeenUsed();
    if (used) return;

    if (event->shouldPropagateTo(&imageInport_, this, source)) {
        imageInport_.propagateEvent(event);
        used |= event->markAsUnused();
    }
    event->setUsed(used);
}

}  // namespace inviwo
