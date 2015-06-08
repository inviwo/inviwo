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

    const vec3 getLookFromMinValue() const { return object_->getLookFromMinValue(); }
    const vec3 getLookFromMaxValue() const { return object_->getLookFromMaxValue(); }

    const vec3 getLookToMinValue() const { return object_->getLookToMinValue(); }
    const vec3 getLookToMaxValue() const { return object_->getLookToMaxValue(); }

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
    enum Direction { UP = 0, LEFT, DOWN, RIGHT };

    vec3 mapNormalizedMousePosToTrackball(const vec2& mousePos, float radius = 1.0f);
    void rotateTrackBall(const vec3 &fromTrackballPos, const vec3 &toTrackballPos);
    dvec3 getBoundedTranslation(const dvec3& lookFrom, const dvec3& lookTo, dvec3 translation);
    double getBoundedZoom(const dvec3& lookFrom, const dvec3& zoomTo, double zoom);

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
    
    T* object_;
    const CameraBase* camera_;
    bool isMouseBeingPressedAndHold_;

    vec2 lastMousePos_;
    double gestureStartNDCDepth_;
    float trackBallWorldSpaceRadius_;

    vec3* lookFrom_;
    vec3* lookTo_;
    vec3* lookUp_;
    
    // Interaction restrictions
    BoolProperty handleInteractionEvents_;
    // Options to restrict translation along view-space axes.
    BoolProperty allowHorizontalPanning_; ///< Enable/disable horizontal panning
    BoolProperty allowVerticalPanning_;   ///< Enable/disable vertical panning
    BoolProperty allowZooming_;           ///< Enable/disable zooming

    // Options to restrict rotation around view-space axes.
    BoolProperty allowHorizontalRotation_;    ///< Enable/disable rotation around horizontal axis
    BoolProperty allowVerticalRotation_;      ///< Enable/disable rotation around vertical axis
    BoolProperty allowViewDirectionRotation_; ///< Enable/disable rotation around view direction axis


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

    float RADIUS = 0.5f; ///< Radius in normalized screen space [0 1]^2
    float STEPSIZE = 0.05f;
};

template <typename T>
Trackball<T>::Trackball(T* object, const CameraBase* camera)
    : CompositeProperty("trackball", "Trackball")
    , object_(object)
    , camera_(camera)
    , isMouseBeingPressedAndHold_(false)
    , lastMousePos_(ivec2(0))
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
        for (size_t i = 0; i < touchEvent->getTouchPoints().size() - 1; ++i) {
            for (size_t j = i + 1; j < touchEvent->getTouchPoints().size(); ++j) {
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

        dvec3 worldSpaceTranslation(getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord));
        
        // Zoom based on the closest point to the object
        // Use the look at point if the closest point is unknown
        auto depth = std::min(touchPoint1->getDepth(), touchPoint2->getDepth());
        if (depth <= -1 || depth >= 1) {
            // Get NDC depth of the lookTo position
            depth = getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(vec2(0.f)).z;
        }
        auto zoomToWorldPos(camera_->getWorldPosFromNormalizedDeviceCoords(vec3(0.f, 0.f, depth)));
        auto direction = zoomToWorldPos - getLookFrom();
        zoom *= glm::length(direction);
        direction = glm::normalize(direction);
        zoom = getBoundedZoom(dvec3(getLookFrom()), dvec3(getLookTo()), zoom);
        vec3 newLookFrom = getLookFrom() + static_cast<float>(zoom)* (direction);
        //LogInfo("New lookFrom: " << newLookFrom << " Direction: " << direction << " Zoom: " << zoom);

        //vec3 boundedWorldSpaceTranslation(getBoundedTranslation(dvec3(newLookFrom), dvec3(getLookTo()), worldSpaceTranslation));
        vec3 boundedWorldSpaceTranslation(getBoundedTranslation(dvec3(newLookFrom), dvec3(getLookTo()), worldSpaceTranslation));
        // Rotating using angle from screen space is equivalent to rotating 
        // around the direction in world space since we are looking into the screen.
        vec3 newLookUp;
        if (allowViewDirectionRotation_) {
            vec3 direction2 = (getLookTo() - getLookFrom());
            newLookUp = glm::normalize(glm::rotate(getLookUp(), static_cast<float>(angle), glm::normalize(direction2)));
        } else {
            newLookUp = getLookUp();
        }
        setLook(newLookFrom - boundedWorldSpaceTranslation, getLookTo() - boundedWorldSpaceTranslation, newLookUp);

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
    
    

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        vec2 normalizedScreenCoord(curMousePos.x, 1.f - curMousePos.y);
        //trackBallWorldSpaceRadius_ = 1.f;// glm::distance(vec2(0), 2.f*normalizedScreenCoord - 1.f);
        lastMousePos_ = curMousePos;
        
        gestureStartNDCDepth_ = mouseEvent->depth();
        //if (gestureStartNDCDepth_ >= 1.) {
        //    gestureStartNDCDepth_ = getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord).z;
        //}
        vec3 curNDCCoord(2.f*normalizedScreenCoord - 1.f, static_cast<float>(gestureStartNDCDepth_));
        trackBallWorldSpaceRadius_ = glm::distance(getLookTo(), camera_->getWorldPosFromNormalizedDeviceCoords(curNDCCoord));

        isMouseBeingPressedAndHold_ = true;

    } else {
        vec2 normalizedDeviceCoord(2.f*curMousePos.x - 1.f, 2.f*(1.f - curMousePos.y) - 1.f);
        vec2 prevNormalizedDeviceCoord(2.f*lastMousePos_.x - 1.f, 2.f*(1.f - lastMousePos_.y) - 1.f);

        vec3 trackballWorldPos;
        vec3 prevTrackballWorldPos;
        bool intersected;
        // Compute coordinates on a sphere to rotate from and to
        {
            float t0 = 0; float t1 = std::numeric_limits<float>::max();
            vec3 o = camera_->getWorldPosFromNormalizedDeviceCoords(vec3(prevNormalizedDeviceCoord, -1.f));
            vec3 d = glm::normalize(camera_->getWorldPosFromNormalizedDeviceCoords(vec3(prevNormalizedDeviceCoord, 0.f)) - o);
            intersected = raySphereIntersection(getLookTo(), trackBallWorldSpaceRadius_, o, d, &t0, &t1);
            prevTrackballWorldPos = o + d*t1;
        }

        {
            float t0 = 0; float t1 = std::numeric_limits<float>::max();
            vec3 o = camera_->getWorldPosFromNormalizedDeviceCoords(vec3(normalizedDeviceCoord, -1.f));
            vec3 d = glm::normalize(camera_->getWorldPosFromNormalizedDeviceCoords(vec3(normalizedDeviceCoord, 0.f)) - o);
            intersected = intersected & raySphereIntersection(getLookTo(), trackBallWorldSpaceRadius_, o, d, &t0, &t1);
            trackballWorldPos = o + d*t1;
        }

        if (intersected && gestureStartNDCDepth_ < 1) {
            vec3 Pa = prevTrackballWorldPos - getLookTo();
            vec3 Pc = trackballWorldPos - getLookTo();
            glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
            setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(), glm::rotate(quaternion, getLookUp()));
        } else {
            
            vec3 prevWorldPos = getLookFrom();
            float rotationAroundVerticalAxis = static_cast<float>((curMousePos.x - lastMousePos_.x)*M_PI);
            float rotationAroundHorizontalAxis = static_cast<float>((curMousePos.y - lastMousePos_.y)*M_PI);

            vec3 Pa = prevWorldPos - getLookTo();

            vec3 Pc = glm::rotate(glm::rotate(Pa, rotationAroundHorizontalAxis, glm::cross(getLookUp(), glm::normalize(getLookFrom() - getLookTo()))), rotationAroundVerticalAxis, getLookUp());
            glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
            setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(), glm::rotate(quaternion, getLookUp()));
            
            
            //rotateTrackBall(lastTrackballPos_, curTrackballPos);
        }

        //update mouse positions
        lastMousePos_ = curMousePos;
    }
}

template <typename T>
void Trackball<T>::zoom(Event* event) {
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    vec2 curMousePos = mouseEvent->posNormalized();
    // compute direction vector
    vec3 direction = getLookFrom() - getLookTo();
    float directionLength = glm::length(direction);
    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        isMouseBeingPressedAndHold_ = true;

    } else if (curMousePos != lastMousePos_ && directionLength > 0) {
        dvec2 normalizedDeviceCoord(2.*curMousePos.x - 1., 2.*(1.f - curMousePos.y) - 1.);
        dvec2 prevNormalizedDeviceCoord(2.*lastMousePos_.x - 1., 2.*(1.f - lastMousePos_.y) - 1.);
        // use the difference in mouse y-position to determine amount of zoom
        double zoom = (normalizedDeviceCoord.y - prevNormalizedDeviceCoord.y)*directionLength;
        // zoom by moving the camera
        if (allowZooming_) {
            zoom = getBoundedZoom(dvec3(getLookFrom()), dvec3(getLookTo()), zoom);

            setLookFrom(getLookFrom() - glm::normalize(direction)*static_cast<float>(zoom));
        }

        lastMousePos_ = curMousePos;
    }
}

template <typename T>
void Trackball<T>::pan(Event* event) {
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    vec2 curMousePos = mouseEvent->posNormalized();
    vec2 normalizedScreenCoord(curMousePos.x, 1.f - curMousePos.y);

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        gestureStartNDCDepth_ = mouseEvent->depth();
        if (gestureStartNDCDepth_ >= 1.) {
            gestureStartNDCDepth_ = getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord).z;
        }
        isMouseBeingPressedAndHold_ = true;

    } else {
        vec3 fromNormalizedDeviceCoord(2.f*lastMousePos_.x - 1.f, 2.f*(1.f - lastMousePos_.y) - 1.f, gestureStartNDCDepth_);
        vec3 toNormalizedDeviceCoord(2.f*normalizedScreenCoord - 1.f, gestureStartNDCDepth_);
        if (!allowHorizontalPanning_)
            toNormalizedDeviceCoord.x = fromNormalizedDeviceCoord.x;
        if (!allowVerticalPanning_)
            toNormalizedDeviceCoord.y = fromNormalizedDeviceCoord.y;

        dvec3 translation(getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord));

        vec3 boundedTranslation(getBoundedTranslation(dvec3(getLookFrom()), dvec3(getLookTo()), translation));
        setLook(getLookFrom() - boundedTranslation, getLookTo() - boundedTranslation, getLookUp());
    }


    lastMousePos_ = curMousePos;
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
    vec3 direction = getLookFrom() - getLookTo();
    double directionLength = glm::length(direction);
    double zoom = 0;
    if (dir == UP)
        zoom = STEPSIZE * directionLength;
    else if (dir == DOWN)
        zoom = -STEPSIZE * directionLength;

    // zoom by moving the camera
    zoom = getBoundedZoom(dvec3(getLookFrom()), dvec3(getLookTo()), zoom);
    setLookFrom(getLookFrom() - glm::normalize(direction)*static_cast<float>(zoom));

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
    dvec3 translation(getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord));
    vec3 boundedTranslation(getBoundedTranslation(dvec3(getLookFrom()), dvec3(getLookTo()), translation));
    setLook(getLookFrom() - boundedTranslation, getLookTo() - boundedTranslation, getLookUp());

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
dvec3 Trackball<T>::getBoundedTranslation(const dvec3& lookFrom, const dvec3& lookTo, dvec3 translation) {
    // Make sure that we do not translate outside of the specified boundaries

    // To avoid sliding motions along boundaries created by clamping we 
    // simply disallow movements that would cause the lookTo point to
    // go out of bounds
    auto newPos(lookTo - translation);
    //auto clampedPos = glm::clamp(newPos, dvec3(getLookToMinValue()), dvec3(getLookToMaxValue()));
    auto distanceToMinBounds = newPos - dvec3(getLookToMinValue());
    auto distanceToMaxBounds = dvec3(getLookToMaxValue()) - newPos;
    auto axesMinDistance = glm::min(distanceToMinBounds, distanceToMaxBounds);
    auto minDistance = glm::min(glm::min(axesMinDistance.y, axesMinDistance.z), axesMinDistance.x);
    // Negative distance means that we would move outside of boundaries
    if (minDistance < 0) {
        translation = dvec3(0);
    } 
    // Clamping does not work when movement is restricted along horizontal or vertical axes.
    //else {
    // 
    //    translation = glm::clamp(translation, dvec3(-axesMinDistance), dvec3(axesMinDistance));
    //}
    //minDistance = minDistance < 0 ? 0 : minDistance;
    //translation = glm::clamp(translation, dvec3(-minDistance), dvec3(minDistance));

    //translation = lookTo - glm::clamp(lookTo - translation, dvec3(getLookToMinValue()), dvec3(getLookToMaxValue()));

    // Currently we cannot enforce the lookFrom boundaries since the zooming does not respect 
    // the lookFrom boundaries. This would create a jerky motion when clamping lookFrom after zooming.
    // TODO: Change this when better lookFrom boundaries have been set.
    //translation = lookFrom - glm::clamp(lookFrom - translation, dvec3(getLookFromMinValue()), dvec3(getLookFromMaxValue()));
    
    // Ensures that the user can rotate around the look to point.
    // This should be resolved using smaller lookTo bounds instead!
    //auto radius = glm::length(lookFrom - lookTo);
    //translation = lookTo - glm::clamp(lookTo - translation, dvec3(getLookFromMinValue()) + radius, dvec3(getLookFromMaxValue()) - radius);
    return translation;
}

template< typename T>
double inviwo::Trackball<T>::getBoundedZoom(const dvec3& lookFrom, const dvec3& zoomTo, double zoom) {



    // Compute the smallest distance between the bounds of lookTo and lookFrom
    auto distanceToMinBounds = glm::abs(dvec3(getLookFromMinValue()) - dvec3(getLookToMinValue()));
    auto distanceToMaxBounds = glm::abs(dvec3(getLookFromMaxValue()) - dvec3(getLookToMaxValue()));
    auto minDistance = glm::min(distanceToMinBounds, distanceToMaxBounds);
    double maxZoomOut;
    auto directionLength = glm::length(lookFrom - zoomTo);
    // Use a heuristic if the lookFrom and lookTo bounds are equal:
    // One cannot zoom out more than half of the smallest bound in xyz.
    // Otherwise:
    // The distance between the lookFrom and lookTo cannot be greater
    // than the smallest distance between the two bounds, 
    // thereby ensuring that the lookFrom and lookTo are inside their bounds.
    if (glm::any(glm::equal(minDistance, dvec3(0)))) {
        // The bounds of lookFrom and lookTo are equal.
        // Using half of the smallest axis will NOT ensure that lookFrom stays 
        // within the bounds but is best for backwards compatibility 
        // (distance between lookFrom and lookTo will be too small otherwise)
        auto lookToBounds = 0.5*(dvec3(getLookToMaxValue()) - dvec3(getLookToMinValue()));
        maxZoomOut = directionLength - glm::min(glm::min(lookToBounds.y, lookToBounds.z), lookToBounds.x);
    } else {
        maxZoomOut = directionLength - glm::min(glm::min(minDistance.y, minDistance.z), minDistance.x);
    }

    // Clamp so that the user does not zoom outside of the bounds and not
    // further than, or onto, the lookTo point.
    zoom = glm::clamp(zoom, maxZoomOut, directionLength - camera_->getNearPlaneDist());
    return zoom;


}

template< typename T>
vec3 inviwo::Trackball<T>::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(const vec2& normalizedScreenCoord) const {
    vec3 normalizedDeviceCoordinate;
    // Default to using focus point for depth
    vec4 lookToClipCoord = camera_->projectionMatrix()*camera_->viewMatrix()*vec4(getLookTo(), 1.f);

    normalizedDeviceCoordinate = vec3(2.f*normalizedScreenCoord - 1.f, lookToClipCoord.z / lookToClipCoord.w);

    return normalizedDeviceCoordinate;
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


