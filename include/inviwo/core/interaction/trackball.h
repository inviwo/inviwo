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
#include <inviwo/core/util/intersection/raysphereintersection.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/datastructures/camera.h>

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
     * const vec3& getLookTo() const;
     * const vec3& getLookFrom() const;
     * const vec3& getLookUp() const;

     * void setLookTo(vec3 lookTo);
     * void setLookFrom(vec3 lookFrom);
     * void setLookUp(vec3 lookUp);
     *
     * void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp);
     * @see CameraTrackball
     */
    Trackball(T* object, const CameraBase* camera);
    virtual ~Trackball();

    virtual void invokeEvent(Event* event) override;

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


    vec3 getWorldSpaceTranslationFromNDCSpace(const vec3& fromNormalizedDeviceCoord, const vec3& toNormalizedDeviceCoord);
    vec3 getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(const vec2& normalizedScreenCoord) const;
protected:
    void setPanSpeedFactor(float psf);

protected:
    enum Direction { UP = 0, LEFT, DOWN, RIGHT };

    vec3 mapNormalizedMousePosToTrackball(const vec2& mousePos, float radius = 1.0f);
    void rotateTrackBall(const vec3 &fromTrackballPos, const vec3 &toTrackballPos);

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

    bool isMouseBeingPressedAndHold_;

    vec2 lastMousePos_;
    vec3 lastTrackballPos_;
    double gestureStartNDCDepth_;

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
    const CameraBase* camera_;

    float RADIUS = 0.5f; ///< Radius in normalized screen space [0 1]^2
    float STEPSIZE = 0.05f;
};

template< typename T>
vec3 inviwo::Trackball<T>::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(const vec2& normalizedScreenCoord) const {
    vec3 normalizedDeviceCoordinate;
    // Default to using focus point for depth
    vec4 lookToClipCoord = camera_->projectionMatrix()*camera_->viewMatrix()*vec4(getLookTo(), 1.f);

    normalizedDeviceCoordinate = vec3(2.f*normalizedScreenCoord - 1.f, lookToClipCoord.z / lookToClipCoord.w);

    return normalizedDeviceCoordinate;
}

template <typename T>
Trackball<T>::Trackball(T* object, const CameraBase* camera)
    : CompositeProperty("trackball", "Trackball")
    , object_(object)
    , camera_(camera)
    , isMouseBeingPressedAndHold_(false)
    , lastMousePos_(ivec2(0))
    , lastTrackballPos_(vec3(0.5f))
    , gestureStartNDCDepth_(-1)
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
    new TouchEvent(),
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
vec3 Trackball<T>::mapNormalizedMousePosToTrackball(const vec2& mousePos, float radius /*= 1.f*/) {
    // set x and y to lie in interval [-r, r]
    float r = radius;
    vec3 result = vec3(2.f*mousePos.x - 1.f, 2.f*(1.f - mousePos.y) - 1.f, 0.0f);

    // Mapping according to Holroyds trackball
    // Piece-wise sphere + hyperbolic sheet
    if ((result.x*result.x + result.y*result.y) <= r*r / (2.0f)) { //Spherical Region
        result.z = r*r - (result.x*result.x + result.y*result.y);
        result.z = result.z > 0.0f ? sqrtf(result.z) : 0.0f;
    } else {  //Hyperbolic Region - for smooth z values
        result.z = ((r*r) / (2.0f*sqrtf(result.x*result.x + result.y*result.y)));
    }

    return result;
}

template< typename T>
vec3 inviwo::Trackball<T>::getWorldSpaceTranslationFromNDCSpace(const vec3& fromNormalizedDeviceCoord, const vec3& toNormalizedDeviceCoord) {

    vec3 prevWorldPos(camera_->getWorldPosFromNormalizedDeviceCoords(vec3(fromNormalizedDeviceCoord)));
    vec3 worldPos(camera_->getWorldPosFromNormalizedDeviceCoords(toNormalizedDeviceCoord));

    vec3 translation = worldPos - prevWorldPos;
    return translation; 
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

        if (touchPoint1->state() == TouchPoint::TOUCH_STATE_STATIONARY || touchPoint2->state() == TouchPoint::TOUCH_STATE_STATIONARY) {
            gestureStartNDCDepth_ = std::min(touchPoint1->getDepth(), touchPoint2->getDepth());
            if (gestureStartNDCDepth_ >= 1.) {
                gestureStartNDCDepth_ = getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(pos1).z;
            }
        }

        // Compute translation in world space
        vec3 fromNormalizedDeviceCoord(2.*prevCenterPoint- 1., gestureStartNDCDepth_);
        vec3 toNormalizedDeviceCoord(2.*centerPoint - 1., gestureStartNDCDepth_);
        if (!allowHorizontalPanning_)
            toNormalizedDeviceCoord.x = fromNormalizedDeviceCoord.x;
        if (!allowVerticalPanning_)
            toNormalizedDeviceCoord.y = fromNormalizedDeviceCoord.y;

        vec3 worldSpaceTranslation = getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord);
        
        // Zoom based on the closest point to the object
        // Use the look at point if the closest point is unknown
        auto depth = std::min(touchPoint1->getDepth(), touchPoint2->getDepth());
        if (depth >= 1) {
            // Get NDC depth of the lookTo position
            depth = getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(vec2(0.f)).z;
        }
        auto zoomToWorldPos(camera_->getWorldPosFromNormalizedDeviceCoords(vec3(0.f, 0.f, depth)));

        vec3 newLookFrom = getLookFrom() + static_cast<float>(zoom) * (zoomToWorldPos-getLookFrom());

        // Rotating using angle from screen space is equivalent to rotating 
        // around the direction in world space since we are looking into the screen.
        vec3 newLookUp;
        if (allowViewDirectionRotation_) {
            vec3 direction = (getLookTo() - getLookFrom());
            newLookUp = glm::normalize(glm::rotate(getLookUp(), static_cast<float>(angle), glm::normalize(direction)));
        } else {
            newLookUp = getLookUp();
        }
        setLook(newLookFrom - worldSpaceTranslation, getLookTo() - worldSpaceTranslation, newLookUp);

        //LogInfo("\nTwo fingers: Scale: " << scale << "\nAngle: " << angle << "\nTranslation: " << worldSpaceTranslation);
        isMouseBeingPressedAndHold_ = false;
        touchEvent->markAsUsed();
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
        vec2 normalizedScreenCoord(curMousePos.x, 1.f - curMousePos.y);
        //vec3 curNDCCoord = screenToWorldTransformer_.getNormalizedDeviceFromNormalizedScreen(normalizedScreenCoord);
        //gestureStartNDCDepth_ = curNDCCoord.z;
        //lookToPressPosWorldSpaceDistance_ = glm::distance(getLookTo(), screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(curNDCCoord));

        isMouseBeingPressedAndHold_ = true;

    } else if (curTrackballPos != lastTrackballPos_) {
        //vec3 worldPos = screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(vec3(curTrackballPos.xy(), curTrackballPos.z - gestureStartNDCDepth_));
        //vec3 prevWorldPos = screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(vec3(lastTrackballPos_.xy(), lastTrackballPos_.z - gestureStartNDCDepth_));
        //{
        //    float t0 = 0; float t1 = std::numeric_limits<float>::max();
        //    vec3 o = screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(vec3(lastTrackballPos_.xy(), -1.f));
        //    vec3 d = glm::normalize(screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(vec3(lastTrackballPos_.xy(), 0.f)) - o);
        //    raySphereIntersection(getLookTo(), lookToPressPosWorldSpaceDistance_, o, d, &t0, &t1);
        //    prevWorldPos = o + d*t1;
        //}

        //{
        //    float t0 = 0; float t1 = std::numeric_limits<float>::max();
        //    vec3 o = screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(vec3(curTrackballPos.xy(), -1.f));
        //    vec3 d = glm::normalize(screenToWorldTransformer_.getWorldPosFromNormalizedDeviceCoords(vec3(curTrackballPos.xy(), 0.f)) - o);
        //    raySphereIntersection(getLookTo(), lookToPressPosWorldSpaceDistance_, o, d, &t0, &t1);
        //    worldPos = o + d*t1;
        //}
        //glm::quat quaternion = glm::angleAxis(static_cast<float>(M_PI)*(lastMousePos_.x - curMousePos.x), getLookUp()) * glm::angleAxis(static_cast<float>(M_PI)*((1.f - curMousePos.y) - (1.f - lastMousePos_.y)), glm::cross(getLookUp(), glm::normalize(getLookFrom() - getLookTo())));
        //vec3 Pa = glm::normalize(prevWorldPos - getLookTo());
        //vec3 Pc = glm::normalize(worldPos - getLookTo());
        //vec3 rotationAxis = glm::cross(Pa, Pc);
        //float angle = atan2(glm::length(rotationAxis), glm::dot(Pa, Pc));
        ////glm::quat quaternion = glm::angleAxis(angle, rotationAxis);

        rotateTrackBall(lastTrackballPos_, curTrackballPos);


        //update mouse positions
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
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


    vec2 normalizedScreenCoord(curMousePos.x, 1.f - curMousePos.y);

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        lastTrackballPos_ = curTrackballPos;
        gestureStartNDCDepth_ = mouseEvent->depth();
        if (gestureStartNDCDepth_ >= 1.) {
            gestureStartNDCDepth_ = getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord).z;
        }
        isMouseBeingPressedAndHold_ = true;

    }
    vec3 fromNormalizedDeviceCoord(2.f*lastMousePos_.x - 1.f, 2.f*(1.f - lastMousePos_.y) - 1.f, gestureStartNDCDepth_);
    vec3 toNormalizedDeviceCoord(2.f*normalizedScreenCoord - 1.f, gestureStartNDCDepth_);
    if (!allowHorizontalPanning_)
        toNormalizedDeviceCoord.x = fromNormalizedDeviceCoord.x;
    if (!allowVerticalPanning_)
        toNormalizedDeviceCoord.y = fromNormalizedDeviceCoord.y;

    vec3 translation = getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord);

    setLook(getLookFrom() - translation, getLookTo() - translation, getLookUp());

    lastMousePos_ = curMousePos;
    lastTrackballPos_ = curTrackballPos;
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
    rotateTrackBall(trackballOrigin, trackballDirection);

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
    vec2 destination = origin;

    switch (dir) {
    case UP:
        destination.y -= STEPSIZE;
        break;

    case LEFT:
        destination.x -= STEPSIZE;
        break;

    case DOWN:
        destination.y += STEPSIZE;
        break;

    case RIGHT:
        destination.x += STEPSIZE;
        break;
    }
    if (!allowHorizontalPanning_)
        destination.x = origin.x;
    if (!allowVerticalPanning_)
        destination.y = origin.y;

    vec3 fromNormalizedDeviceCoord(getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(origin));
    vec3 toNormalizedDeviceCoord(2.f*destination - 1.f, fromNormalizedDeviceCoord.z);
    vec3 translation = getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord);
    setLook(getLookFrom() - translation, getLookTo() - translation, getLookUp());

}

template <typename T>
void Trackball<T>::rotateTrackBall(const vec3 &fromTrackBallPos, const vec3 &toTrackBallPos) {
    vec3 view = glm::normalize(getLookFrom() - getLookTo());
    vec3 right = glm::cross(getLookUp(), view);
    // Transform virtual sphere coordinates to view space
    vec3 Pa = fromTrackBallPos.x*right + fromTrackBallPos.y * getLookUp() + fromTrackBallPos.z*view;
    vec3 Pc = toTrackBallPos.x*right + toTrackBallPos.y * getLookUp() + toTrackBallPos.z*view;
    // Compute the rotation that transforms coordinates
    glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
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
