/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/interaction/trackball.h>
#include <glm/gtx/vector_angle.hpp>

namespace inviwo {
static const float RADIUS = 0.5f;
static const float STEPSIZE = 0.05f;

PropertyClassIdentifier(Trackball, "org.inviwo.Trackball");

Trackball::Trackball(vec3* lookFrom, vec3* lookTo, vec3* lookUp)
    : CompositeProperty("trackball", "Trackball")
    , pixelWidth_(0.007f)
    , isMouseBeingPressedAndHold_(false)
    , lastMousePos_(ivec2(0))
    , lastTrackballPos_(vec3(0.5f))
    , lookFrom_(lookFrom)
    , lookTo_(lookTo)
    , lookUp_(lookUp)
    
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true,
                               VALID)

    , mouseRotate_("trackballRotate", "Rotate",
        new MouseEvent(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_NONE,
            MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
        new Action(this, &Trackball::rotate))
    
    , mouseZoom_("trackballZoom", "Zoom",
        new MouseEvent(MouseEvent::MOUSE_BUTTON_RIGHT, InteractionEvent::MODIFIER_NONE,
            MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
        new Action(this, &Trackball::zoom))

    , mousePan_("trackballPan", "Pan", 
        new  MouseEvent(MouseEvent::MOUSE_BUTTON_MIDDLE, InteractionEvent::MODIFIER_NONE,
            MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
        new Action(this, &Trackball::pan))

    , mouseReset_("mouseReset", "Reset",
        new  MouseEvent(MouseEvent::MOUSE_BUTTON_ANY, InteractionEvent::MODIFIER_NONE,
            MouseEvent::MOUSE_STATE_RELEASE),
        new Action(this, &Trackball::reset))

    , stepRotateUp_("stepRotateUp", "Rotate up", 
        new KeyboardEvent('W', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::rotateUp))

    , stepRotateLeft_("stepRotateLeft", "Rotate left",
        new KeyboardEvent('A', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::rotateLeft))

    , stepRotateDown_("stepRotateDown", "Rotate down",
        new KeyboardEvent('S', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::rotateDown))

    , stepRotateRight_("stepRotateRight", "Rotate right", 
        new KeyboardEvent('D', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::rotateRight))

    , stepZoomIn_("stepZoomIn", "Zoom in",
        new KeyboardEvent('R', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::zoomIn))

    , stepZoomOut_("stepZoomOut", "Zoom out",
        new KeyboardEvent('F', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::zoomOut))

    , stepPanUp_("stepPanUp", "Pan up", 
        new KeyboardEvent('W', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::panUp))

    , stepPanLeft_("stepPanLeft", "Pan left",
        new KeyboardEvent('A', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::panLeft))

    , stepPanDown_("stepPanDown", "Pan down", 
        new KeyboardEvent('S', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::panDown))

    , stepPanRight_("stepPanRight", "Pan right", 
        new KeyboardEvent('D', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
        new Action(this, &Trackball::panRight))
    
    , pinchGesture_("pinchGesture", "Pinch",
        new GestureEvent(GestureEvent::PINCH, GestureEvent::GESTURE_STATE_ANY, 2),
        new Action(this, &Trackball::pinchGesture))
    
    , panGesture_("panGesture", "Pan",
        new GestureEvent(GestureEvent::PAN, GestureEvent::GESTURE_STATE_ANY, 3),
        new Action(this, &Trackball::panGesture)) {
        

    mouseReset_.setVisible(false);
    mouseReset_.setCurrentStateAsDefault();
    pinchGesture_.setVisible(false);
    pinchGesture_.setCurrentStateAsDefault();
    panGesture_.setVisible(false);
    panGesture_.setCurrentStateAsDefault();

    addProperty(handleInteractionEvents_);

    addProperty(mouseRotate_);
    addProperty(mouseZoom_);
    addProperty(mousePan_);
    addProperty(mouseReset_);
    addProperty(stepRotateUp_);
    addProperty(stepRotateLeft_);
    addProperty(stepRotateDown_);
    addProperty(stepRotateRight_);
    addProperty(stepZoomIn_);
    addProperty(stepZoomOut_);
    addProperty(stepPanUp_);
    addProperty(stepPanLeft_);
    addProperty(stepPanDown_);
    addProperty(stepPanRight_);
    addProperty(pinchGesture_);
    addProperty(panGesture_);
}

Trackball::~Trackball() {}

void Trackball::invokeInteractionEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    CompositeProperty::invokeInteractionEvent(event);
}

vec3 Trackball::mapNormalizedMousePosToTrackball(const vec2& mousePos, float dist /*= 1.f*/) {
    // set x and y to lie in interval [-r, r]
    float r = RADIUS;
    vec3 result = vec3(mousePos.x-RADIUS, -1.0f*(mousePos.y-RADIUS), 0.0f)*dist;

    // Mapping according to Holroyds trackball
    // Piece-wise sphere + hyperbolic sheet
    if ((result.x*result.x + result.y*result.y) <= r*r/(2.0f)) { //Spherical Region
        result.z = r*r - (result.x*result.x + result.y*result.y);
        result.z = result.z > 0.0f ? sqrtf(result.z) : 0.0f;
    } else {  //Hyperbolic Region - for smooth z values
        result.z = ((r*r)/(2.0f*sqrtf(result.x*result.x + result.y*result.y)));
    }

    return glm::normalize(result);
}

vec3 Trackball::mapToObject(vec3 pos, float dist) {
    // return (camera_->viewMatrix() * vec4(pos,0)).xyz;
    // TODO: Use proper co-ordinate transformation matrices
    // Get x,y,z axis vectors of current camera view
    vec3 currentViewYaxis = glm::normalize(*lookUp_);
    vec3 currentViewZaxis = glm::normalize(*lookFrom_-*lookTo_);
    vec3 currentViewXaxis = glm::normalize(glm::cross(currentViewYaxis, currentViewZaxis));

    //mapping to camera co-ordinate
    currentViewXaxis*=pos.x*dist;
    currentViewYaxis*=pos.y*dist;
    currentViewZaxis*=pos.z*dist;
    return (currentViewXaxis + currentViewYaxis + currentViewZaxis);
}

void Trackball::pinchGesture(Event* event) {
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    vec3 direction = *lookFrom_ - *lookTo_;
    float vecLength = glm::clamp(glm::length(direction), 0.5f, 4.f);
    vec3 normdirection = glm::normalize(direction);
    *lookFrom_ = *lookFrom_ -
                 normdirection * (static_cast<float>(vecLength * gestureEvent->deltaDistance()));
    notifyLookFromChanged(this);
    isMouseBeingPressedAndHold_ = false;
}

void Trackball::panGesture(Event* event) {
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    vec3 offsetVector =
        vec3(gestureEvent->deltaPos().x * 2.f, gestureEvent->deltaPos().y * 2.f, 0.f);

    // The resulting rotation needs to be mapped to the camera distance,
    // as if the trackball is located at a certain distance from the camera.
    // TODO: Verify this
    float zDist = (glm::length(*lookFrom_ - *lookTo_) - 1.f) / M_PI;
    vec3 mappedOffsetVector = mapToObject(offsetVector, zDist);

    *lookTo_ += mappedOffsetVector;
    *lookFrom_ += mappedOffsetVector;
    notifyAllChanged(this);
    isMouseBeingPressedAndHold_ = false;
}

void Trackball::rotate(Event* event) {  
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    
    vec2 curMousePos = mouseEvent->posNormalized();

    // The resulting rotation needs to be mapped to the camera distance,
    // as if the trackball is located at a certain distance from the camera.
    // TODO: Verify this
    // float zDist = (glm::length(*lookFrom_-*lookTo_)-1.f)/M_PI;
    // vec3 curTrackballPos = mapNormalizedMousePosToTrackball(curMousePos, zDist);

    vec3 curTrackballPos = mapNormalizedMousePosToTrackball(curMousePos);

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastTrackballPos_ = curTrackballPos;
        lastMousePos_ = curMousePos;
        isMouseBeingPressedAndHold_ = true;
    
    } else if (curTrackballPos != lastTrackballPos_) {
        // calculate rotation angle (in radians)
        float rotationAngle = glm::angle(curTrackballPos, lastTrackballPos_);
        //difference vector in trackball co-ordinates
        vec3 trackBallOffsetVector = lastTrackballPos_ - curTrackballPos;
        //compute next camera position
        vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector);
        vec3 currentCamPos = *lookFrom_;
        vec3 nextCamPos = currentCamPos + mappedTrackBallOffsetVector;

        // obtain rotation axis
        if (glm::degrees(rotationAngle) > pixelWidth_) {
            rotateFromPosToPos(currentCamPos, nextCamPos, rotationAngle);

            //update mouse positions
            lastMousePos_ = curMousePos;
            lastTrackballPos_ = curTrackballPos;
        }
    }
}

void Trackball::zoom(Event* event) {
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    float diff;
    vec2 curMousePos = mouseEvent->posNormalized();
    vec3 curTrackballPos = mapNormalizedMousePosToTrackball(curMousePos);
    // compute direction vector
    vec3 direction = *lookFrom_ - *lookTo_;

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
        isMouseBeingPressedAndHold_ = true;
    
    } else if (curMousePos != lastMousePos_ && direction.length() > 0) {
        // use the difference in mouse y-position to determine amount of zoom
        diff = curTrackballPos.y - lastTrackballPos_.y;
        // zoom by moving the camera
        *lookFrom_ -= direction*diff;
        notifyLookFromChanged(this);
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
    }
}

void Trackball::pan(Event* event) {
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    vec2 curMousePos = mouseEvent->posNormalized();
    vec3 curTrackballPos = mapNormalizedMousePosToTrackball(curMousePos);

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
        isMouseBeingPressedAndHold_ = true;
    }

    // difference vector in trackball co-ordinates
    vec3 trackBallOffsetVector = lastTrackballPos_ - curTrackballPos;
    // compute next camera position
    trackBallOffsetVector.z = 0.0f;

    //The resulting rotation needs to be mapped to the camera distance,
    //as if the trackball is located at a certain distance from the camera.
    //TODO: Verify this
    //float zDist = (glm::length(*lookFrom_-*lookTo_)-1.f)/M_PI;
    //vec3 mappedTrackBallOffsetVector = mapToCamera(trackBallOffsetVector, zDist);

    vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector);

    if (curMousePos != lastMousePos_) {
        *lookTo_ += mappedTrackBallOffsetVector;
        *lookFrom_ += mappedTrackBallOffsetVector;
        notifyAllChanged(this);
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
    }
}

void Trackball::stepRotate(Direction dir) {
    vec2 origin = vec2(0.5, 0.5);
    vec2 direction = origin;

    switch (dir) {
    case UP:
        direction.y -= STEPSIZE;
        break;

    case LEFT:
        direction.x -= STEPSIZE;
        break;

    case DOWN:
        direction.y += STEPSIZE;
        break;

    case RIGHT:
        direction.x += STEPSIZE;
        break;
    }

    vec3 trackballDirection = mapNormalizedMousePosToTrackball(direction);
    vec3 trackballOrigin = mapNormalizedMousePosToTrackball(origin);
    // calculate rotation angle (in degrees)
    float rotationAngle = glm::angle(trackballDirection, trackballOrigin);
    //difference vector in trackball co-ordinates
    vec3 trackBallOffsetVector = trackballOrigin - trackballDirection;
    //compute next camera position
    vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector);
    vec3 currentCamPos = *lookFrom_;
    vec3 nextCamPos = currentCamPos + mappedTrackBallOffsetVector;

    // obtain rotation axis
    if (glm::degrees(rotationAngle) > pixelWidth_) {
        rotateFromPosToPos(currentCamPos, nextCamPos, rotationAngle);
    }
}

void Trackball::stepZoom(Direction dir) {
    // compute direction vector
    vec3 direction = vec3(0);

    if (dir == UP)
        direction = *lookFrom_ - *lookTo_;
    else if (dir == DOWN)
        direction = *lookTo_ - *lookFrom_;

    // zoom by moving the camera
    *lookFrom_ -= direction*STEPSIZE;
    notifyLookFromChanged(this);
}

void Trackball::stepPan(Direction dir) {
    vec2 origin = vec2(0.5, 0.5);
    vec2 direction = origin;

    switch (dir) {
    case UP:
        direction.y -= STEPSIZE;
        break;

    case LEFT:
        direction.x -= STEPSIZE;
        break;

    case DOWN:
        direction.y += STEPSIZE;
        break;

    case RIGHT:
        direction.x += STEPSIZE;
        break;
    }

    //vec2 curMousePos = mouseEvent->posNormalized();
    vec3 trackballDirection = mapNormalizedMousePosToTrackball(direction);
    vec3 trackballOrigin = mapNormalizedMousePosToTrackball(origin);
    //difference vector in trackball co-ordinates
    vec3 trackBallOffsetVector = trackballOrigin - trackballDirection;
    //compute next camera position
    trackBallOffsetVector.z = 0.0f;
    vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector);
    *lookTo_  += mappedTrackBallOffsetVector;
    *lookFrom_ += mappedTrackBallOffsetVector;
    notifyAllChanged(this);
}

void Trackball::rotateFromPosToPos(const vec3& currentCamPos, const vec3& nextCamPos,
                                   float rotationAngle) {
    // rotation axis
    vec3 rotationAxis = glm::cross(currentCamPos, nextCamPos);
    // generate quaternion and rotate camera
    rotationAxis = glm::normalize(rotationAxis);
    quat quaternion = glm::angleAxis(rotationAngle, rotationAxis);
    // float lookLength = glm::length(*lookFrom_-*lookTo_);
    *lookFrom_ = *lookTo_ + glm::rotate(quaternion, *lookFrom_ - *lookTo_);
    *lookUp_ = glm::rotate(quaternion, *lookUp_);

    // Check the length of the length-vector, might change due to float precision
    // vec3 direction = *lookFrom_-*lookTo_;
    // float newLookLength = glm::length(direction);
    // if (lookLength != newLookLength) {
    //    float diff = newLookLength- lookLength;
    //    *lookFrom_ += glm::normalize(direction) * diff;
    // }

    notifyAllChanged(this);
}

void Trackball::rotateLeft(Event* event) {
    stepRotate(LEFT);
}

void Trackball::rotateRight(Event* event) {
    stepRotate(RIGHT);
}

void Trackball::rotateUp(Event* event) {
    stepRotate(UP);
}

void Trackball::rotateDown(Event* event) {
    stepRotate(DOWN);
}

void Trackball::panLeft(Event* event) {
    stepPan(LEFT);
}

void Trackball::panRight(Event* event) {
    stepPan(RIGHT);
}

void Trackball::panUp(Event* event) {
    stepPan(UP);
}

void Trackball::panDown(Event* event) {
    stepPan(DOWN);
}

void Trackball::zoomIn(Event* event) {
    stepZoom(UP);
}

void Trackball::zoomOut(Event* event) {
    stepZoom(DOWN);
}

void Trackball::reset(Event* event) {
    isMouseBeingPressedAndHold_ = false;
}


void TrackballObservable::notifyAllChanged(const Trackball* trackball) const {
    // Notify observers
    for (ObserverSet::iterator it = observers_->begin(), itEnd = observers_->end(); it != itEnd;
         ++it) {
        // static_cast can be used since only template class objects can be added
        static_cast<TrackballObserver*>(*it)->onAllTrackballChanged(trackball);
    }
}

void TrackballObservable::notifyLookFromChanged(const Trackball* trackball) const {
    for (ObserverSet::iterator it = observers_->begin(), itEnd = observers_->end(); it != itEnd;
         ++it) {
        static_cast<TrackballObserver*>(*it)->onLookFromChanged(trackball);
    }
}

void TrackballObservable::notifyLookToChanged(const Trackball* trackball) const {
    for (ObserverSet::iterator it = observers_->begin(), itEnd = observers_->end(); it != itEnd;
         ++it) {
        static_cast<TrackballObserver*>(*it)->onLookToChanged(trackball);
    }
}

void TrackballObservable::notifyLookUpChanged(const Trackball* trackball) const {
    for (ObserverSet::iterator it = observers_->begin(), itEnd = observers_->end(); it != itEnd;
         ++it) {
        static_cast<TrackballObserver*>(*it)->onLookUpChanged(trackball);
    }
}

TrackballObservable::TrackballObservable() : Observable<TrackballObserver>() {

}

} // namespace