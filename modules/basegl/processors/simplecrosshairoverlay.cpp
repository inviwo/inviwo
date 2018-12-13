/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/basegl/processors/simplecrosshairoverlay.h>

#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

const ProcessorInfo SimpleCrosshairOverlay::processorInfo_{
    "org.inviwo.SimpleCrosshairOverlay", // Class identifier
    "Simple Crosshair Overlay",          // Display name
    "Overlay",                           // Category
    CodeState::Experimental,             // Code state
    Tags::GL,                            // Tags
};
const ProcessorInfo SimpleCrosshairOverlay::getProcessorInfo() const { return processorInfo_; }

SimpleCrosshairOverlay::SimpleCrosshairOverlay()
    : Processor()
    , imageIn_("imageIn")
    , imageOut_("imageOut")
    /*, planePosition_("planePosition", "Plane Position", vec3(0.0f), vec3(-1.0f), vec3(1.0f))
    , planeNormal_("planeNormal", "Plane Normal", vec3(0.0f), vec3(-1.0f), vec3(1.0f))
    , planeUp_("planeUp", "Plane Up", vec3(0.0f), vec3(-1.0f), vec3(1.0f))*/
    , cursorAngle_("cursorAngle", "Cursor Angle", 0.0f, 0.0f, glm::two_pi<float>())
    , cursorPos_("cursorPos", "Cursor Position", vec2(0.5f), vec2(0.0f), vec2(1.0f))
    , cursorRadius_("cursorRadius", "Cursor Radius", 0.1f, 0.0f, 1.0f)
    , mouseEvent_("mouseEvent", "Mouse Event", [this](Event* e) { updateCrosshair(e); },
                  MouseButton::Left, MouseState::Press | MouseState::Move | MouseState::Release)
    , lastMousePos_(0.0f)
    , interactionState_ (InteractionState::NONE) {

    imageIn_.setOptional(true);
    addPort(imageIn_);
    addPort(imageOut_);

    /*addProperty(planePosition_);
    addProperty(planeNormal_);
    addProperty(planeUp_);*/

    addProperty(cursorAngle_);
    addProperty(cursorPos_);
    addProperty(cursorRadius_);

    addProperty(mouseEvent_);
}

void SimpleCrosshairOverlay::updateCrosshair(Event* e) {
    const auto mouseEvent = static_cast<MouseEvent*>(e);
    const auto mouseState = mouseEvent->state();
    const auto newMousePos = vec2(mouseEvent->posNormalized());

    if (mouseState == MouseState::Press) { // ### update last mouse position at mouse down ###
        lastMousePos_ = newMousePos;

        if (glm::distance(newMousePos, cursorPos_.get()) < cursorRadius_) { // ### determine if cursor rotation or movement
            interactionState_ = InteractionState::MOVE;
        } else {
            interactionState_ = InteractionState::ROTATE;
        }
    } else if (mouseState == MouseState::Move) {
        if (interactionState_ == InteractionState::ROTATE) { // ### update angle at mouse move ###
            // angle between cursor center to old and new mouse pos
            const auto dirOld = glm::normalize(lastMousePos_ - cursorPos_.get());
            const auto dirNew = glm::normalize(newMousePos - cursorPos_.get());
            const auto angleDiff = glm::acos(glm::dot(dirOld, dirNew));

            // determine direction of rotation
            const float angleSign = (glm::cross(vec3(dirOld, 0.0f), vec3(dirNew, 0.0f)).z > 0) ? 1.0f : -1.0f;

            // update angle in temporary variable
            auto tmp = cursorAngle_.get();
            tmp += angleSign * angleDiff;

            // fix angle between 0 and 2pi
            while (tmp > glm::two_pi<float>()) {
                tmp -= glm::two_pi<float>();
            }
            while (tmp < 0.0f) {
                tmp += glm::two_pi<float>();
            }

            // set new angle [0..2pi]
            cursorAngle_ = tmp;

            // update regular mouse pos while rotating
            lastMousePos_ = newMousePos;
        } else if (interactionState_ == InteractionState::MOVE) { // ### update position at mouse move ###
            const auto newMousePosClamped = glm::clamp(newMousePos, vec2(0.0f), vec2(1.0f));
            const auto diff = newMousePosClamped - lastMousePos_;
            const auto newCursorPos = glm::clamp(cursorPos_.get() + diff, vec2(0.0f), vec2(1.0f));

            // set new position vec2[0..1]
            cursorPos_ = newCursorPos;

            // update clamped new mouse pos while dragging
            lastMousePos_ = newMousePosClamped;
        }
    } else if (mouseState == MouseState::Release) {
        interactionState_ = InteractionState::NONE;
    }
}

}  // namespace inviwo
