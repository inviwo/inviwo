/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <inviwo/core/interaction/twoaxisvaluatortrackball.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>

#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/epsilon.hpp>

namespace inviwo {

const std::string TwoAxisValuatorTrackball::classIdentifier = "org.inviwo.TwoAxisValuatorTrackball";
std::string TwoAxisValuatorTrackball::getClassIdentifier() const { return classIdentifier; }

TwoAxisValuatorTrackball::TwoAxisValuatorTrackball(CameraProperty* camProp)
: Trackball(camProp)
, sensitivity_("Sensitivity", "Sensitivity", 5.0f, 0.0f, 10.0f, 0.1f)
, v_angle_limit_("v_angle_limit", "Vertical Angle Limit", 0.09f, 0.0f, glm::pi<float>()/2.0f, 0.001f)
, fixUp_("FixUp", "Fix Up Vector", false)
{
    mouseZoom_ = EventProperty("trackballZoom", "Zoom", [this](Event* e) { zoomWheel(e); }, util::make_unique<WheelEventMatcher>());
    mouseRotate_.setAction([&](Event* e){ rotate(e); });
    addProperty(sensitivity_);
    addProperty(fixUp_);
    addProperty(v_angle_limit_);
}

/* \brief Checks for epsilon equality with floats (Pretty high standard epsilon!)
*/
bool epsEq(float a, float b, float eps=3e-3) { return abs(a - b) < eps; }


/* \brief Zooms on mouse wheel event
 *
 * Uses the step zoom functions and triggers one
 * step per mouse wheel tick. Horizontal scrolling is disregarded
 */
void TwoAxisValuatorTrackball::zoomWheel(Event* event){
    auto wheelEvent = static_cast<WheelEvent*>(event);
    int steps = static_cast<int>(wheelEvent->delta().y);

    for(size_t i=0; i<abs(steps); ++i)
        if(steps > 0) zoomIn(event);
        else zoomOut(event);
}

/* \brief rotates the trackball on mouse drag event. Uses Two Axis Valuator method.
 *
 * Horizontal mouse movement is mapped to rotation around the camera up vector (world up if fixUp_)
 * and vertical mouse movement is mapped to rotation around the camera right vector.
 * The trackball's sensivity_ property adjusts speed. The vertical rotation can be restricted
 * using the v_angle_limit_ property which limits the angle between world up and the view direction
 * by restricting the camera position accordingly.
 */
void TwoAxisValuatorTrackball::rotate(Event* event) {
    auto mouseEvent = static_cast<MouseEvent*>(event);
    const auto ndc = static_cast<vec3>(mouseEvent->ndc());

    const auto curNDC =
        vec3(allowHorizontalRotation_ ? ndc.x : 0.0f, allowVerticalRotation_ ? ndc.y : 0.0f,
             followObjectDuringRotation_ ? ndc.z : 1);

    const auto& to = this->getLookTo();
    const auto& from = this->getLookFrom();
    const vec3 w_up = vec3(0.f, 1.f, 0.f);
    const vec3 up = fixUp_ ? w_up : this->getLookUp();
    const auto& right = glm::cross(glm::normalize(to - from), up);
    const float dist = glm::length(from - to);
    const vec3 from_dir = glm::normalize(from - to);

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        isMouseBeingPressedAndHold_ = true;
        gestureStartNDCDepth_ = curNDC.z;
        trackBallWorldSpaceRadius_ =
            glm::distance(to, object_->getWorldPosFromNormalizedDeviceCoords(curNDC));
    } else {
        // Compute coordinates on a sphere to rotate from and to
        vec2 diff = glm::xy(curNDC - this->lastNDC_);
        float correct_height = cos(v_angle_limit_);
        if(fixUp_) { // If fixUp, prevent skipping over poles
            float va = acos(dot(from_dir, w_up));
            if(diff.y < 0.0f && epsEq(va,                    v_angle_limit_) ||
               diff.y > 0.0f && epsEq(glm::pi<float>() - va, v_angle_limit_)) {
                diff.y = 0;
            }
        }
        if(glm::all(glm::epsilonEqual(diff, vec2(0.f), vec2(1e-6)))) { event->markAsUsed(); return; }
        const vec2 diff_n = diff / (abs(diff.x) + abs(diff.y)); // P1 Normalize

        const vec3 rot_ax = glm::normalize(-diff_n.x * up + diff_n.y * right);
        const vec3 new_from = glm::rotate(glm::normalize(from - to), glm::length(diff) * sensitivity_, rot_ax);
        const vec3 new_from_dir = glm::normalize(new_from - to);
        vec3 new_up;
        vec3 correct_from = new_from;

        if(fixUp_){
            new_up = glm::cross(glm::cross(to - new_from, up), to - new_from);
            float height = glm::dot(new_from_dir, w_up);

            if(abs(height) > correct_height){ // Camera too close to poles
                vec3 h_dir = glm::normalize(new_from_dir - height * w_up);

                float correct_rad = sqrt(1 - pow(correct_height, 2.0f));
                correct_from = glm::normalize(h_dir * correct_rad + w_up * height);
            }
        } else {
            new_up = glm::rotate(up, glm::length(diff) * sensitivity_, rot_ax);
        }

        setLook(to + correct_from * dist, to, new_up);
    }
    // update mouse positions
    this->lastNDC_ = curNDC;
    event->markAsUsed();
}

}  // namespace inviwo
