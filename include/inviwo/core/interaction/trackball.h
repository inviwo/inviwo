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

#ifndef IVW_TRACKBALL_H
#define IVW_TRACKBALL_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/action.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/util/observer.h>

#include <glm/gtx/vector_angle.hpp>
#include <math.h>

namespace inviwo {

template< typename T>
class Trackball : public CompositeProperty {
public:

    /**
     * Rotates and moves object around a sphere.
     * This object does not take ownership of pointers handed to it.
     * The template class is expected to have the following functions:
     * vec3& getLookTo();
     * vec3& getLookFrom();
     * vec3& getLookUp();
     * const vec3& getLookTo() const;
     * const vec3& getLookFrom() const;
     * const vec3& getLookUp() const;

     * void setLookTo(vec3 lookTo);
     * void setLookFrom(vec3 lookFrom);
     * void setLookUp(vec3 lookUp);
     *
     * void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp);
     * @CameraTrackball
     */
    Trackball(T* object);
    virtual ~Trackball();

    virtual void invokeEvent(Event* event) override;

    vec3& getLookTo() { return object_->getLookTo(); }
    vec3& getLookFrom() { return object_->getLookFrom(); }
    vec3& getLookUp() { return object_->getLookUp(); }
    const vec3& getLookTo() const { return object_->getLookTo(); }
    const vec3& getLookFrom() const { return object_->getLookFrom(); }
    const vec3& getLookUp() const { return object_->getLookUp(); }

    void setLookTo(vec3 lookTo) { object_->setLookTo(lookTo); }
    void setLookFrom(vec3 lookFrom) { object_->setLookFrom(lookFrom); }
    void setLookUp(vec3 lookUp) { object_->setLookUp(lookUp); }

    /** 
     * \brief Set look from, look to and up vector at the same time. 
     * Should be used when more than one parameter will be changed to avoid duplicate evaluations.
     * 
     * @param vec3 lookFrom 
     * @param vec3 lookTo 
     * @param vec3 lookUp 
     */
    void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) { object_->setLook(lookFrom, lookTo, lookUp); }

protected:
    void setPanSpeedFactor(float psf);

protected:
    enum Direction { UP = 0, LEFT, DOWN, RIGHT };

    vec3 mapNormalizedMousePosToTrackball(const vec2& mousePos, float dist = 1.f);
    vec3 mapToObject(vec3 pos, float dist = 1.f);
    void rotateFromPosToPos(const vec3& currentCamPos, const vec3& nextCamPos, float rotationAngle);

    void rotate(Event* event);
    void zoom(Event* event);
    void pan(Event* event);
    void reset(Event* event);

    void stepRotate(Direction dir);
    void stepZoom(Direction dir);
    void stepPan(Direction dir);

    void rotateLeft(Event* event);
    void rotateRight(Event* event);
    void rotateUp(Event* event);
    void rotateDown(Event* event);

    void panLeft(Event* event);
    void panRight(Event* event);
    void panUp(Event* event);
    void panDown(Event* event);

    void zoomIn(Event* event);
    void zoomOut(Event* event);
    
    /** 
     * \brief Rotates around the direction vector, 
     * zooms along the direction vector and translates along up/right vector.
     * 
     * @param Event * event TouchEvent
     */
    void touchGesture(Event* event);

    float pixelWidth_;
    float panSpeedFactor_;
    bool isMouseBeingPressedAndHold_;

    vec2 lastMousePos_;
    vec3 lastTrackballPos_;

    vec3* lookFrom_;
    vec3* lookTo_;
    vec3* lookUp_;
    
    // Interaction restrictions
    // Options to restrict translation along view-space axes.
    BoolProperty allowHorizontalPanning_; ///< Enable/disable horizontal panning
    BoolProperty allowVerticalPanning_;   ///< Enable/disable vertical panning
    BoolProperty allowZooming_;           ///< Enable/disable zooming

    // Options to restrict rotation around view-space axes.
    BoolProperty allowHorizontalRotation_;    ///< Enable/disable rotation around horizontal axis
    BoolProperty allowVerticalRotation_;      ///< Enable/disable rotation around vertical axis
    BoolProperty allowViewDirectionRotation_; ///< Enable/disable rotation around view direction axis

    BoolProperty handleInteractionEvents_;

    // Event Properties.
    EventProperty mouseRotate_;
    EventProperty mouseZoom_;
    EventProperty mousePan_;
    EventProperty mouseReset_;

    EventProperty stepRotateUp_;
    EventProperty stepRotateLeft_;
    EventProperty stepRotateDown_;
    EventProperty stepRotateRight_;

    EventProperty stepZoomIn_;
    EventProperty stepZoomOut_;
    EventProperty stepPanUp_;
    EventProperty stepPanLeft_;
    EventProperty stepPanDown_;
    EventProperty stepPanRight_;

    EventProperty touchGesture_;

    T* object_;

    float RADIUS = 0.5f; ///< Radius in normalized screen space [0 1]^2
    float STEPSIZE = 0.05f;
};

template <typename T>
Trackball<T>::Trackball(T* object)
    : CompositeProperty("trackball", "Trackball")
    , object_(object)
    , pixelWidth_(0.007f)
    , panSpeedFactor_(1.f)
    , isMouseBeingPressedAndHold_(false)
    , lastMousePos_(ivec2(0))
    , lastTrackballPos_(vec3(0.5f))
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true,
    VALID)

    , allowHorizontalPanning_("allowHorizontalPanning", "Horizontal panning enabled", true)
    , allowVerticalPanning_("allowVerticalPanning", "Vertical panning enabled", true)
    , allowZooming_("allowZoom", "Zoom enabled", true)

    , allowHorizontalRotation_("allowHorziontalRotation", "Rotation around horizontal axis", true)
    , allowVerticalRotation_("allowVerticalRotation", "Rotation around vertical axis", true)
    , allowViewDirectionRotation_("allowViewAxisRotation", "Rotation around view axis", true)

    , mouseRotate_("trackballRotate", "Rotate",
    new MouseEvent(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_NONE,
    MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
    new Action(this, &Trackball<T>::rotate))

    , mouseZoom_("trackballZoom", "Zoom",
    new MouseEvent(MouseEvent::MOUSE_BUTTON_RIGHT, InteractionEvent::MODIFIER_NONE,
    MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
    new Action(this, &Trackball<T>::zoom))

    , mousePan_("trackballPan", "Pan",
    new  MouseEvent(MouseEvent::MOUSE_BUTTON_MIDDLE, InteractionEvent::MODIFIER_NONE,
    MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
    new Action(this, &Trackball<T>::pan))

    , mouseReset_("mouseReset", "Reset",
    new  MouseEvent(MouseEvent::MOUSE_BUTTON_ANY, InteractionEvent::MODIFIER_NONE,
    MouseEvent::MOUSE_STATE_RELEASE),
    new Action(this, &Trackball<T>::reset))

    , stepRotateUp_("stepRotateUp", "Rotate up",
    new KeyboardEvent('W', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::rotateUp))

    , stepRotateLeft_("stepRotateLeft", "Rotate left",
    new KeyboardEvent('A', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::rotateLeft))

    , stepRotateDown_("stepRotateDown", "Rotate down",
    new KeyboardEvent('S', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::rotateDown))

    , stepRotateRight_("stepRotateRight", "Rotate right",
    new KeyboardEvent('D', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::rotateRight))

    , stepZoomIn_("stepZoomIn", "Zoom in",
    new KeyboardEvent('R', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::zoomIn))

    , stepZoomOut_("stepZoomOut", "Zoom out",
    new KeyboardEvent('F', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::zoomOut))

    , stepPanUp_("stepPanUp", "Pan up",
    new KeyboardEvent('W', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::panUp))

    , stepPanLeft_("stepPanLeft", "Pan left",
    new KeyboardEvent('A', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::panLeft))

    , stepPanDown_("stepPanDown", "Pan down",
    new KeyboardEvent('S', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::panDown))

    , stepPanRight_("stepPanRight", "Pan right",
    new KeyboardEvent('D', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &Trackball<T>::panRight))

    , touchGesture_("touchGesture", "Touch",
    new TouchEvent(TouchEvent::TOUCH_STATE_ANY),
    new Action(this, &Trackball<T>::touchGesture))
{


    mouseReset_.setVisible(false);
    mouseReset_.setCurrentStateAsDefault();

    addProperty(handleInteractionEvents_);

    addProperty(allowHorizontalPanning_);
    addProperty(allowVerticalPanning_);
    addProperty(allowZooming_);

    addProperty(allowHorizontalRotation_);
    addProperty(allowVerticalRotation_);
    addProperty(allowViewDirectionRotation_);

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

    addProperty(touchGesture_);
    touchGesture_.setVisible(false); // No options to change button combination to trigger event

    setCollapsed(true);
}

template <typename T>
Trackball<T>::~Trackball() {}

template <typename T>
void Trackball<T>::invokeEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    CompositeProperty::invokeEvent(event);
}

template <typename T>
void Trackball<T>::setPanSpeedFactor(float psf) {
    panSpeedFactor_ = psf;
}

template <typename T>
vec3 Trackball<T>::mapNormalizedMousePosToTrackball(const vec2& mousePos, float dist /*= 1.f*/) {
    // set x and y to lie in interval [-r, r]
    float r = RADIUS;
    vec3 result = vec3(mousePos.x - RADIUS, -1.0f*(mousePos.y - RADIUS), 0.0f)*dist;

    // Mapping according to Holroyds trackball
    // Piece-wise sphere + hyperbolic sheet
    if ((result.x*result.x + result.y*result.y) <= r*r / (2.0f)) { //Spherical Region
        result.z = r*r - (result.x*result.x + result.y*result.y);
        result.z = result.z > 0.0f ? sqrtf(result.z) : 0.0f;
    } else {  //Hyperbolic Region - for smooth z values
        result.z = ((r*r) / (2.0f*sqrtf(result.x*result.x + result.y*result.y)));
    }

    return glm::normalize(result);
}

template <typename T>
vec3 Trackball<T>::mapToObject(vec3 pos, float dist) {
    // return (camera_->viewMatrix() * vec4(pos,0)).xyz;
    // TODO: Use proper co-ordinate transformation matrices
    // Get x,y,z axis vectors of current camera view
    vec3 currentViewYaxis = glm::normalize(getLookUp());
    vec3 currentViewZaxis = glm::normalize(getLookFrom() - getLookTo());
    vec3 currentViewXaxis = glm::normalize(glm::cross(currentViewYaxis, currentViewZaxis));

    //mapping to camera co-ordinate
    currentViewXaxis *= pos.x*dist;
    currentViewYaxis *= pos.y*dist;
    currentViewZaxis *= pos.z*dist;
    return (currentViewXaxis + currentViewYaxis + currentViewZaxis);
}

template <typename T>
void Trackball<T>::touchGesture(Event* event) {
    TouchEvent* touchEvent = static_cast<TouchEvent*>(event);

    if (touchEvent->getTouchPoints().size() > 1) {
        // Use the two closest points to extract translation, scaling and rotation
        auto touchPoints = touchEvent->getTouchPoints();
        const TouchPoint* touchPoint1 = &touchPoints[0]; const TouchPoint* touchPoint2 = &touchPoints[1];
        float distance = std::numeric_limits<float>::max();
        for (auto i = 0; i < touchEvent->getTouchPoints().size() - 1; ++i) {
            for (auto j = i + 1; j < touchEvent->getTouchPoints().size(); ++j) {
                float ijDistance = glm::distance2(touchPoints[i].getPos(), touchPoints[j].getPos());
                if (ijDistance < distance) {
                    distance = ijDistance;
                    touchPoint1 = &touchPoints[i];
                    touchPoint2 = &touchPoints[j];
                }
            }
        }

        // Flip y-position to get coordinate system
        // (0, 1)--(1, 1)
        //   |        |
        // (0, 0)--(1, 0)
        dvec2 prevPos1 = touchPoint1->getPrevPosNormalized(); prevPos1.y = 1. - prevPos1.y;
        dvec2 prevPos2 = touchPoint2->getPrevPosNormalized(); prevPos2.y = 1. - prevPos2.y;
        dvec2 pos1 = touchPoint1->getPosNormalized(); pos1.y = 1. - pos1.y;
        dvec2 pos2 = touchPoint2->getPosNormalized(); pos2.y = 1. - pos2.y;

        auto v1(prevPos2 - prevPos1);
        auto v2(pos2 - pos1);

        // The glm implementation of orientedAngle returns positive angle 
        // for small negative angles. 
        //auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
        // Roll our own angle calculation
        auto v1Normalized = glm::normalize(v1);
        auto v2Normalized = glm::normalize(v2);
        auto angle = acos(glm::clamp(glm::dot(v1Normalized, v2Normalized), -1., 1.));
        // Check orientation using cross product.
        if (v1Normalized.x*v2Normalized.y - v2Normalized.x*v1Normalized.y < 0) {
            angle = -angle;
        }

        auto zoom = 1 - glm::length(v1) / glm::length(v2);
        if (!std::isfinite(zoom) || !allowZooming_) {
            zoom = 0;
        }
        // Difference between midpoints before and after
        auto prevCenterPoint = glm::mix(prevPos1, prevPos2, 0.5);
        auto centerPoint = glm::mix(pos1, pos2, 0.5);

        // Compute translation in world space
        // We currently do not have the information about the scene 
        // so we need to rely on the mapToObject function
        //vec4 lookToClipCoord = cameraProp_->projectionMatrix()*cameraProp_->viewMatrix()*vec4(cameraProp_->getLookTo(), 1.f);
        //vec3 lookToNDCCoord = vec3(lookToClipCoord.xyz() / lookToClipCoord.w);
        //float ndcDepth = lookToNDCCoord.z;
        //if (!std::isfinite(lookToClipCoord.w)) {
        //    ndcDepth = 1;
        //}
        //vec3 worldSpaceTranslation = cameraProp_->getWorldPosFromNormalizedDeviceCoords(vec3(-1.f + 2.f*centerPoint, ndcDepth)) - cameraProp_->getWorldPosFromNormalizedDeviceCoords(vec3(-1.f + 2.f*prevCenterPoint, ndcDepth));

        vec3 direction = (getLookTo() - getLookFrom());
        dvec2 allowTranslation(allowHorizontalPanning_ ? 1 : 0, allowVerticalPanning_ ? 1 : 0);
        vec3 worldSpaceTranslation = mapToObject(vec3(centerPoint*allowTranslation, 0)*panSpeedFactor_, glm::length(direction)) - mapToObject(vec3(prevCenterPoint*allowTranslation, 0)*panSpeedFactor_, glm::length(direction));

        // Zoom based on the closest point to the object
        // Use the look at point if the closest point is unknown
        vec3 newLookFrom = getLookFrom() + static_cast<float>(zoom)* direction;
        // Rotating using angle from screen space is equivalent to rotating 
        // around the direction in world space since we are looking into the screen.
        vec3 newLookUp;
        if (allowViewDirectionRotation_) {
            newLookUp = glm::normalize(glm::rotate(getLookUp(), static_cast<float>(angle), glm::normalize(direction)));
        } else {
            newLookUp = getLookUp();
        }
        setLook(newLookFrom - worldSpaceTranslation, getLookTo() - worldSpaceTranslation, newLookUp);

        //LogInfo("\nTwo fingers: Scale: " << scale << "\nAngle: " << angle << "\nTranslation: " << worldSpaceTranslation);
        isMouseBeingPressedAndHold_ = false;
    }

}

template <typename T>
void Trackball<T>::rotate(Event* event) {
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    vec2 curMousePos = mouseEvent->posNormalized();
    if (!allowHorizontalRotation_) {
        curMousePos.y = 0.5;
    }
    if (!allowVerticalRotation_) {
        curMousePos.x = 0.5;
    }
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
        vec3 currentCamPos = getLookFrom();
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

template <typename T>
void Trackball<T>::zoom(Event* event) {
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    float diff;
    vec2 curMousePos = mouseEvent->posNormalized();
    vec3 curTrackballPos = mapNormalizedMousePosToTrackball(curMousePos);
    // compute direction vector
    vec3 direction = getLookFrom() - getLookTo();

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
        isMouseBeingPressedAndHold_ = true;

    } else if (curMousePos != lastMousePos_ && direction.length() > 0) {
        // use the difference in mouse y-position to determine amount of zoom
        diff = (curTrackballPos.y - lastTrackballPos_.y);
        // zoom by moving the camera
        if (allowZooming_)
            setLookFrom(getLookFrom() - direction*diff);
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
    }
}

template <typename T>
void Trackball<T>::pan(Event* event) {
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
    vec3 trackBallOffsetVector = vec3(lastMousePos_ - curMousePos, 0.f);

    trackBallOffsetVector *= panSpeedFactor_;

    trackBallOffsetVector.y = -trackBallOffsetVector.y;
    if (!allowHorizontalPanning_)
        trackBallOffsetVector.x = 0;
    if (!allowVerticalPanning_)
        trackBallOffsetVector.y = 0;

    float ratio = (float)mouseEvent->canvasSize().x
        / (float)mouseEvent->canvasSize().y;
    vec2 screenScale = vec2(1.f);

    if (ratio > 1.f)
        screenScale.x = ratio;
    else if (ratio < 1.f)
        screenScale.y = 1.f / ratio;

    trackBallOffsetVector.x *= screenScale.x;
    trackBallOffsetVector.y *= screenScale.y;

    vec3 direction = getLookFrom() - getLookTo();
    float vecLength = glm::length(direction);
    vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector, vecLength);

    if (curMousePos != lastMousePos_) {
        setLook(getLookFrom() + mappedTrackBallOffsetVector, getLookTo() + mappedTrackBallOffsetVector, getLookUp());

        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
    }
}

template <typename T>
void Trackball<T>::stepRotate(Direction dir) {
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
    if (!allowHorizontalRotation_)
        direction.y = origin.y;
    if (!allowVerticalRotation_)
        direction.x = origin.x;

    vec3 trackballDirection = mapNormalizedMousePosToTrackball(direction);
    vec3 trackballOrigin = mapNormalizedMousePosToTrackball(origin);
    // calculate rotation angle (in degrees)
    float rotationAngle = glm::angle(trackballDirection, trackballOrigin);
    //difference vector in trackball co-ordinates
    vec3 trackBallOffsetVector = trackballOrigin - trackballDirection;
    //compute next camera position
    vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector);
    vec3 currentCamPos = getLookFrom();
    vec3 nextCamPos = currentCamPos + mappedTrackBallOffsetVector;

    // obtain rotation axis
    if (glm::degrees(rotationAngle) > pixelWidth_) {
        rotateFromPosToPos(currentCamPos, nextCamPos, rotationAngle);
    }
}

template <typename T>
void Trackball<T>::stepZoom(Direction dir) {
    if (!allowZooming_) {
        return;
    }
    // compute direction vector
    vec3 direction = vec3(0);

    if (dir == UP)
        direction = getLookFrom() - getLookTo();
    else if (dir == DOWN)
        direction = getLookTo() - getLookFrom();

    // zoom by moving the camera
    setLookFrom(getLookFrom() - direction*STEPSIZE);

}

template <typename T>
void Trackball<T>::stepPan(Direction dir) {
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
    if (!allowHorizontalPanning_)
        direction.x = origin.x;
    if (!allowVerticalPanning_)
        direction.y = origin.y;
    //vec2 curMousePos = mouseEvent->posNormalized();
    vec3 trackballDirection = mapNormalizedMousePosToTrackball(direction);
    vec3 trackballOrigin = mapNormalizedMousePosToTrackball(origin);
    //difference vector in trackball co-ordinates
    vec3 trackBallOffsetVector = trackballOrigin - trackballDirection;
    //compute next camera position
    trackBallOffsetVector.z = 0.0f;
    vec3 mappedTrackBallOffsetVector = mapToObject(trackBallOffsetVector);
    setLook(getLookFrom() + mappedTrackBallOffsetVector, getLookTo() + mappedTrackBallOffsetVector, getLookUp());

}

template <typename T>
void Trackball<T>::rotateFromPosToPos(const vec3& currentCamPos, const vec3& nextCamPos,
    float rotationAngle) {
    // rotation axis
    vec3 rotationAxis = glm::cross(currentCamPos, nextCamPos);
    // generate quaternion and rotate camera
    rotationAxis = glm::normalize(rotationAxis);
    quat quaternion = glm::angleAxis(rotationAngle, rotationAxis);
    
    setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(), glm::rotate(quaternion, getLookUp()));
}

template <typename T>
void Trackball<T>::rotateLeft(Event* event) {
    stepRotate(LEFT);
}

template <typename T>
void Trackball<T>::rotateRight(Event* event) {
    stepRotate(RIGHT);
}

template <typename T>
void Trackball<T>::rotateUp(Event* event) {
    stepRotate(UP);
}

template <typename T>
void Trackball<T>::rotateDown(Event* event) {
    stepRotate(DOWN);
}

template <typename T>
void Trackball<T>::panLeft(Event* event) {
    stepPan(LEFT);
}

template <typename T>
void Trackball<T>::panRight(Event* event) {
    stepPan(RIGHT);
}

template <typename T>
void Trackball<T>::panUp(Event* event) {
    stepPan(UP);
}

template <typename T>
void Trackball<T>::panDown(Event* event) {
    stepPan(DOWN);
}

template <typename T>
void Trackball<T>::zoomIn(Event* event) {
    stepZoom(UP);
}

template <typename T>
void Trackball<T>::zoomOut(Event* event) {
    stepZoom(DOWN);
}

template <typename T>
void Trackball<T>::reset(Event* event) {
    isMouseBeingPressedAndHold_ = false;
}

}

#endif  // IVW_TRACKBALL_H