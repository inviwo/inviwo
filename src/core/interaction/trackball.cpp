/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

namespace inviwo {

Trackball::Trackball(TrackballObject* object)
    : CompositeProperty("trackball", "Trackball")
    , object_(object)
    , isMouseBeingPressedAndHold_(false)
    , lastMousePos_(ivec2(0))
    , gestureStartNDCDepth_(-1)
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true,
                               InvalidationLevel::Valid)

    , allowHorizontalPanning_("allowHorizontalPanning", "Horizontal panning enabled", true)
    , allowVerticalPanning_("allowVerticalPanning", "Vertical panning enabled", true)
    , allowZooming_("allowZoom", "Zoom enabled", true)
    , maxZoomInDistance_("minDistanceToLookAtPoint", "Minimum zoom distance", 0., 0, 1000)

    , allowHorizontalRotation_("allowHorziontalRotation", "Rotation around horizontal axis", true)
    , allowVerticalRotation_("allowVerticalRotation", "Rotation around vertical axis", true)
    , allowViewDirectionRotation_("allowViewAxisRotation", "Rotation around view axis", true)
    , animate_("animate", "Animate rotations", false)
    , mouseRotate_("trackballRotate", "Rotate",
                   new MouseEvent(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_NONE,
                                  MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
                   new Action(this, &Trackball::rotate))

    , mouseZoom_("trackballZoom", "Zoom",
                 new MouseEvent(MouseEvent::MOUSE_BUTTON_RIGHT, InteractionEvent::MODIFIER_NONE,
                                MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
                 new Action(this, &Trackball::zoom))

    , mousePan_("trackballPan", "Pan",
                new MouseEvent(MouseEvent::MOUSE_BUTTON_MIDDLE, InteractionEvent::MODIFIER_NONE,
                               MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
                new Action(this, &Trackball::pan))

    , mouseReset_("mouseReset", "Reset",
                  new MouseEvent(MouseEvent::MOUSE_BUTTON_ANY, InteractionEvent::MODIFIER_ANY,
                                 MouseEvent::MOUSE_STATE_RELEASE),
                  new Action(this, &Trackball::reset))

    , stepRotateUp_(
          "stepRotateUp", "Rotate up",
          new KeyboardEvent('W', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::rotateUp))

    , stepRotateLeft_(
          "stepRotateLeft", "Rotate left",
          new KeyboardEvent('A', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::rotateLeft))

    , stepRotateDown_(
          "stepRotateDown", "Rotate down",
          new KeyboardEvent('S', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::rotateDown))

    , stepRotateRight_(
          "stepRotateRight", "Rotate right",
          new KeyboardEvent('D', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::rotateRight))

    , stepZoomIn_("stepZoomIn", "Zoom in", new KeyboardEvent('R', InteractionEvent::MODIFIER_NONE,
                                                             KeyboardEvent::KEY_STATE_PRESS),
                  new Action(this, &Trackball::zoomIn))

    , stepZoomOut_(
          "stepZoomOut", "Zoom out",
          new KeyboardEvent('F', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::zoomOut))

    , stepPanUp_("stepPanUp", "Pan up", new KeyboardEvent('W', InteractionEvent::MODIFIER_SHIFT,
                                                          KeyboardEvent::KEY_STATE_PRESS),
                 new Action(this, &Trackball::panUp))

    , stepPanLeft_(
          "stepPanLeft", "Pan left",
          new KeyboardEvent('A', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::panLeft))

    , stepPanDown_(
          "stepPanDown", "Pan down",
          new KeyboardEvent('S', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::panDown))

    , stepPanRight_(
          "stepPanRight", "Pan right",
          new KeyboardEvent('D', InteractionEvent::MODIFIER_SHIFT, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &Trackball::panRight))

    , touchGesture_("touchGesture", "Touch", new TouchEvent(),
                    new Action(this, &Trackball::touchGesture))
    , evaluated_(true)
    , timer_(30, [this]() { animate(); }) {
    mouseReset_.setVisible(false);
    mouseReset_.setCurrentStateAsDefault();

    addProperty(handleInteractionEvents_);

    addProperty(allowHorizontalPanning_);
    addProperty(allowVerticalPanning_);
    addProperty(allowZooming_);

    addProperty(allowHorizontalRotation_);
    addProperty(allowVerticalRotation_);
    addProperty(allowViewDirectionRotation_);
    addProperty(animate_);

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
    touchGesture_.setVisible(false);  // No options to change button combination to trigger event

    setCollapsed(true);
}

Trackball::Trackball(const Trackball& rhs)
    : CompositeProperty(rhs)
    , object_(rhs.object_)
    , isMouseBeingPressedAndHold_(false)
    , lastMousePos_(ivec2(0))
    , gestureStartNDCDepth_(-1)
    , handleInteractionEvents_(rhs.handleInteractionEvents_)
    , allowHorizontalPanning_(rhs.allowHorizontalPanning_)
    , allowVerticalPanning_(rhs.allowVerticalPanning_)
    , allowZooming_(rhs.allowZooming_)
    , maxZoomInDistance_(rhs.maxZoomInDistance_)
    , allowHorizontalRotation_(rhs.allowHorizontalRotation_)
    , allowVerticalRotation_(rhs.allowVerticalRotation_)
    , allowViewDirectionRotation_(rhs.allowViewDirectionRotation_)
    , animate_(rhs.animate_)
    , mouseRotate_(rhs.mouseRotate_)
    , mouseZoom_(rhs.mouseZoom_)
    , mousePan_(rhs.mousePan_)
    , mouseReset_(rhs.mouseReset_)
    , stepRotateUp_(rhs.stepRotateUp_)
    , stepRotateLeft_(rhs.stepRotateLeft_)
    , stepRotateDown_(rhs.stepRotateDown_)
    , stepRotateRight_(rhs.stepRotateRight_)
    , stepZoomIn_(rhs.stepZoomIn_)
    , stepZoomOut_(rhs.stepZoomOut_)
    , stepPanUp_(rhs.stepPanUp_)
    , stepPanLeft_(rhs.stepPanLeft_)
    , stepPanDown_(rhs.stepPanDown_)
    , stepPanRight_(rhs.stepPanRight_)
    , touchGesture_(rhs.touchGesture_)
    , evaluated_(true)
    , timer_(30, [this]() { animate(); }) {
    mouseReset_.setVisible(false);
    mouseReset_.setCurrentStateAsDefault();

    addProperty(handleInteractionEvents_);

    addProperty(allowHorizontalPanning_);
    addProperty(allowVerticalPanning_);
    addProperty(allowZooming_);
    addProperty(maxZoomInDistance_);

    addProperty(allowHorizontalRotation_);
    addProperty(allowVerticalRotation_);
    addProperty(allowViewDirectionRotation_);
    addProperty(animate_);

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
    touchGesture_.setVisible(false);  // No options to change button combination to trigger event

    setCollapsed(true);
}

Trackball& Trackball::operator=(const Trackball& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        object_ = that.object_;
        isMouseBeingPressedAndHold_ = false;
        lastMousePos_ = ivec2(0);
        gestureStartNDCDepth_ = -1;
        handleInteractionEvents_ = that.handleInteractionEvents_;
        allowHorizontalPanning_ = that.allowHorizontalPanning_;
        allowVerticalPanning_ = that.allowVerticalPanning_;
        allowZooming_ = that.allowZooming_;
        allowHorizontalRotation_ = that.allowHorizontalRotation_;
        allowVerticalRotation_ = that.allowVerticalRotation_;
        allowViewDirectionRotation_ = that.allowViewDirectionRotation_;
        animate_ = that.animate_;
        mouseRotate_ = that.mouseRotate_;
        mouseZoom_ = that.mouseZoom_;
        mousePan_ = that.mousePan_;
        mouseReset_ = that.mouseReset_;
        stepRotateUp_ = that.stepRotateUp_;
        stepRotateLeft_ = that.stepRotateLeft_;
        stepRotateDown_ = that.stepRotateDown_;
        stepRotateRight_ = that.stepRotateRight_;
        stepZoomIn_ = that.stepZoomIn_;
        stepZoomOut_ = that.stepZoomOut_;
        stepPanUp_ = that.stepPanUp_;
        stepPanLeft_ = that.stepPanLeft_;
        stepPanDown_ = that.stepPanDown_;
        stepPanRight_ = that.stepPanRight_;
        touchGesture_ = that.touchGesture_;
    }
    return *this;
}

Trackball::~Trackball() {}

void Trackball::invokeEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    CompositeProperty::invokeEvent(event);
}

const vec3& Trackball::getLookTo() const { return object_->getLookTo(); }

vec3 Trackball::mapNormalizedMousePosToTrackball(const vec2& mousePos, float radius /*= 1.f*/) {
    // set x and y to lie in interval [-r, r]
    float r = radius;
    vec3 result = vec3(2.f * mousePos.x - 1.f, 2.f * (1.f - mousePos.y) - 1.f, 0.0f);

    // Mapping according to Holroyds trackball
    // Piece-wise sphere + hyperbolic sheet
    if ((result.x * result.x + result.y * result.y) <= r * r / (2.0f)) {  // Spherical Region
        result.z = r * r - (result.x * result.x + result.y * result.y);
        result.z = result.z > 0.0f ? sqrtf(result.z) : 0.0f;
    } else {  // Hyperbolic Region - for smooth z values
        result.z = ((r * r) / (2.0f * sqrtf(result.x * result.x + result.y * result.y)));
    }

    return result;
}

vec3 Trackball::getWorldSpaceTranslationFromNDCSpace(const vec3& fromNormalizedDeviceCoord,
                                                     const vec3& toNormalizedDeviceCoord) {
    vec3 prevWorldPos(
        object_->getWorldPosFromNormalizedDeviceCoords(vec3(fromNormalizedDeviceCoord)));
    vec3 worldPos(object_->getWorldPosFromNormalizedDeviceCoords(toNormalizedDeviceCoord));

    vec3 translation = worldPos - prevWorldPos;
    return translation;
}

void Trackball::touchGesture(Event* event) {
    TouchEvent* touchEvent = static_cast<TouchEvent*>(event);

    // Use the two closest points to extract translation, scaling and rotation
    const std::vector<TouchPoint> touchPoints = touchEvent->getTouchPoints();

    if (touchPoints.size() > 1) {
        const TouchPoint touchPoint1 = touchPoints[0];
        const TouchPoint touchPoint2 = touchPoints[1];

        // Flip y-position to get coordinate system
        // (0, 1)--(1, 1)
        //   |        |
        // (0, 0)--(1, 0)
        dvec2 prevPos1 = touchPoint1.getPrevPosNormalized();
        prevPos1.y = 1. - prevPos1.y;
        dvec2 prevPos2 = touchPoint2.getPrevPosNormalized();
        prevPos2.y = 1. - prevPos2.y;
        dvec2 pos1 = touchPoint1.getPosNormalized();
        pos1.y = 1. - pos1.y;
        dvec2 pos2 = touchPoint2.getPosNormalized();
        pos2.y = 1. - pos2.y;

        auto v1(prevPos2 - prevPos1);
        auto v2(pos2 - pos1);

        // The glm implementation of orientedAngle returns positive angle
        // for small negative angles.
        // auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
        // Roll our own angle calculation
        auto v1Normalized = glm::normalize(v1);
        auto v2Normalized = glm::normalize(v2);
        auto angle = acos(glm::clamp(glm::dot(v1Normalized, v2Normalized), -1., 1.));
        // Check orientation using cross product.
        if (v1Normalized.x * v2Normalized.y - v2Normalized.x * v1Normalized.y < 0) {
            angle = -angle;
        }

        auto zoom = 1 - glm::length(v1) / glm::length(v2);

        if (!std::isfinite(zoom) || !allowZooming_) {
            zoom = 0;
        }
        // Difference between midpoints before and after
        auto prevCenterPoint = glm::mix(prevPos1, prevPos2, 0.5);
        auto centerPoint = glm::mix(pos1, pos2, 0.5);

        if (touchPoint1.state() == TouchPoint::TOUCH_STATE_STATIONARY ||
            touchPoint2.state() == TouchPoint::TOUCH_STATE_STATIONARY) {
            gestureStartNDCDepth_ = std::min(touchPoint1.getDepth(), touchPoint2.getDepth());
            if (gestureStartNDCDepth_ >= 1.) {
                gestureStartNDCDepth_ =
                    object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(pos1).z;
            }
        }

        // Compute translation in world space
        vec3 fromNormalizedDeviceCoord(2. * prevCenterPoint - 1., gestureStartNDCDepth_);
        vec3 toNormalizedDeviceCoord(2. * centerPoint - 1., gestureStartNDCDepth_);
        if (!allowHorizontalPanning_) toNormalizedDeviceCoord.x = fromNormalizedDeviceCoord.x;
        if (!allowVerticalPanning_) toNormalizedDeviceCoord.y = fromNormalizedDeviceCoord.y;

        dvec3 worldSpaceTranslation(getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord,
                                                                         toNormalizedDeviceCoord));

        // Zoom based on the closest point to the object
        // Use the look at point if the closest point is unknown
        auto depth = std::min(touchPoint1.getDepth(), touchPoint2.getDepth());
        if (depth <= -1 || depth >= 1) {
            // Get NDC depth of the lookTo position
            depth = object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(vec2(0.f)).z;
        }
        auto zoomToWorldPos(object_->getWorldPosFromNormalizedDeviceCoords(vec3(0.f, 0.f, depth)));
        auto direction = zoomToWorldPos - getLookFrom();
        zoom *= glm::length(direction);
        direction = glm::normalize(direction);
        zoom = getBoundedZoom(dvec3(getLookFrom()), dvec3(getLookTo()), zoom);
        vec3 newLookFrom = getLookFrom() + static_cast<float>(zoom) * (direction);
        // LogInfo("New lookFrom: " << newLookFrom << " Direction: " << direction << " Zoom: " <<
        // zoom);

        // vec3 boundedWorldSpaceTranslation(getBoundedTranslation(dvec3(newLookFrom),
        // dvec3(getLookTo()), worldSpaceTranslation));
        vec3 boundedWorldSpaceTranslation(
            getBoundedTranslation(dvec3(newLookFrom), dvec3(getLookTo()), worldSpaceTranslation));
        // Rotating using angle from screen space is equivalent to rotating
        // around the direction in world space since we are looking into the screen.
        vec3 newLookUp;
        if (allowViewDirectionRotation_) {
            vec3 direction2 = (getLookTo() - getLookFrom());
            newLookUp = glm::normalize(
                glm::rotate(getLookUp(), static_cast<float>(angle), glm::normalize(direction2)));
        } else {
            newLookUp = getLookUp();
        }
        setLook(newLookFrom - boundedWorldSpaceTranslation,
                getLookTo() - boundedWorldSpaceTranslation, newLookUp);

        // LogInfo("\nTwo fingers: Scale: " << scale << "\nAngle: " << angle << "\nTranslation: " <<
        // worldSpaceTranslation);
        isMouseBeingPressedAndHold_ = false;
        touchEvent->markAsUsed();
    }
}

void Trackball::rotate(Event* event) {
    timer_.stop();
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
        // trackBallWorldSpaceRadius_ = 1.f;// glm::distance(vec2(0), 2.f*normalizedScreenCoord -
        // 1.f);
        lastMousePos_ = curMousePos;

        gestureStartNDCDepth_ = mouseEvent->depth();
        // if (gestureStartNDCDepth_ >= 1.) {
        //    gestureStartNDCDepth_ =
        //    getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord).z;
        //}
        vec3 curNDCCoord(2.f * normalizedScreenCoord - 1.f,
                         static_cast<float>(gestureStartNDCDepth_));
        trackBallWorldSpaceRadius_ =
            glm::distance(getLookTo(), object_->getWorldPosFromNormalizedDeviceCoords(curNDCCoord));

        isMouseBeingPressedAndHold_ = true;

    } else {
        vec2 normalizedDeviceCoord(2.f * curMousePos.x - 1.f, 2.f * (1.f - curMousePos.y) - 1.f);
        vec2 prevNormalizedDeviceCoord(2.f * lastMousePos_.x - 1.f,
                                       2.f * (1.f - lastMousePos_.y) - 1.f);

        vec3 trackballWorldPos;
        vec3 prevTrackballWorldPos;
        bool intersected;
        // Compute coordinates on a sphere to rotate from and to
        {
            float t0 = 0;
            float t1 = std::numeric_limits<float>::max();
            vec3 o = object_->getWorldPosFromNormalizedDeviceCoords(
                vec3(prevNormalizedDeviceCoord, -1.f));
            vec3 d = glm::normalize(object_->getWorldPosFromNormalizedDeviceCoords(
                                        vec3(prevNormalizedDeviceCoord, 0.f)) -
                                    o);
            intersected =
                raySphereIntersection(getLookTo(), trackBallWorldSpaceRadius_, o, d, &t0, &t1);
            prevTrackballWorldPos = o + d * t1;
        }

        {
            float t0 = 0;
            float t1 = std::numeric_limits<float>::max();
            vec3 o =
                object_->getWorldPosFromNormalizedDeviceCoords(vec3(normalizedDeviceCoord, -1.f));
            vec3 d = glm::normalize(
                object_->getWorldPosFromNormalizedDeviceCoords(vec3(normalizedDeviceCoord, 0.f)) -
                o);
            intersected =
                intersected &
                raySphereIntersection(getLookTo(), trackBallWorldSpaceRadius_, o, d, &t0, &t1);
            trackballWorldPos = o + d * t1;
        }

        if (intersected && gestureStartNDCDepth_ < 1) {
            vec3 Pa = prevTrackballWorldPos - getLookTo();
            vec3 Pc = trackballWorldPos - getLookTo();
            glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
            lastRot_ = quaternion;
            lastRotTime_ = std::chrono::system_clock::now();
            setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(),
                    glm::rotate(quaternion, getLookUp()));
        } else {
            vec3 prevWorldPos = getLookFrom();
            float rotationAroundVerticalAxis =
                static_cast<float>((curMousePos.x - lastMousePos_.x) * M_PI);
            float rotationAroundHorizontalAxis =
                static_cast<float>((curMousePos.y - lastMousePos_.y) * M_PI);

            vec3 Pa = prevWorldPos - getLookTo();

            vec3 Pc = glm::rotate(
                glm::rotate(Pa, rotationAroundHorizontalAxis,
                            glm::cross(getLookUp(), glm::normalize(getLookFrom() - getLookTo()))),
                rotationAroundVerticalAxis, getLookUp());
            glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
            lastRot_ = quaternion;
            lastRotTime_ = std::chrono::system_clock::now();
            setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(),
                    glm::rotate(quaternion, getLookUp()));

            // rotateTrackBall(lastTrackballPos_, curTrackballPos);
        }

        // update mouse positions
        lastMousePos_ = curMousePos;
    }
    event->markAsUsed();
}

void Trackball::zoom(Event* event) {
    timer_.stop();
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
        dvec2 normalizedDeviceCoord(2. * curMousePos.x - 1., 2. * (1.f - curMousePos.y) - 1.);
        dvec2 prevNormalizedDeviceCoord(2. * lastMousePos_.x - 1.,
                                        2. * (1.f - lastMousePos_.y) - 1.);
        // use the difference in mouse y-position to determine amount of zoom
        double zoom = (normalizedDeviceCoord.y - prevNormalizedDeviceCoord.y) * directionLength;
        // zoom by moving the camera
        if (allowZooming_) {
            zoom = getBoundedZoom(dvec3(getLookFrom()), dvec3(getLookTo()), zoom);

            setLookFrom(getLookFrom() - glm::normalize(direction) * static_cast<float>(zoom));
        }

        lastMousePos_ = curMousePos;
    }
    event->markAsUsed();
}

void Trackball::pan(Event* event) {
    timer_.stop();
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);

    vec2 curMousePos = mouseEvent->posNormalized();
    vec2 normalizedScreenCoord(curMousePos.x, 1.f - curMousePos.y);

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        lastMousePos_ = curMousePos;
        gestureStartNDCDepth_ = mouseEvent->depth();
        if (gestureStartNDCDepth_ >= 1.) {
            gestureStartNDCDepth_ =
                object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord).z;
        }
        isMouseBeingPressedAndHold_ = true;

    } else {
        vec3 fromNormalizedDeviceCoord(2.f * lastMousePos_.x - 1.f,
                                       2.f * (1.f - lastMousePos_.y) - 1.f, gestureStartNDCDepth_);
        vec3 toNormalizedDeviceCoord(2.f * normalizedScreenCoord - 1.f, gestureStartNDCDepth_);
        if (!allowHorizontalPanning_) toNormalizedDeviceCoord.x = fromNormalizedDeviceCoord.x;
        if (!allowVerticalPanning_) toNormalizedDeviceCoord.y = fromNormalizedDeviceCoord.y;

        dvec3 translation(getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord,
                                                               toNormalizedDeviceCoord));

        vec3 boundedTranslation(
            getBoundedTranslation(dvec3(getLookFrom()), dvec3(getLookTo()), translation));
        setLook(getLookFrom() - boundedTranslation, getLookTo() - boundedTranslation, getLookUp());
    }

    lastMousePos_ = curMousePos;
    event->markAsUsed();
}

void Trackball::stepRotate(Direction dir) {
    vec2 origin = vec2(0.5, 0.5);
    vec2 direction = origin;

    switch (dir) {
        case Direction::Up:
            direction.y -= stepsize;
            break;

        case Direction::Left:
            direction.x -= stepsize;
            break;

        case Direction::Down:
            direction.y += stepsize;
            break;

        case Direction::Right:
            direction.x += stepsize;
            break;
    }
    if (!allowHorizontalRotation_) direction.y = origin.y;
    if (!allowVerticalRotation_) direction.x = origin.x;

    vec3 trackballDirection = mapNormalizedMousePosToTrackball(direction);
    vec3 trackballOrigin = mapNormalizedMousePosToTrackball(origin);
    rotateTrackBall(trackballOrigin, trackballDirection);
}

void Trackball::stepZoom(Direction dir) {
    if (!allowZooming_) {
        return;
    }
    // compute direction vector
    vec3 direction = getLookFrom() - getLookTo();
    double directionLength = glm::length(direction);
    double zoom = 0;
    if (dir == Direction::Up) {
        zoom = stepsize * directionLength;
    } else if (dir == Direction::Down) {
        zoom = -stepsize * directionLength;
    }

    // zoom by moving the camera
    zoom = getBoundedZoom(dvec3(getLookFrom()), dvec3(getLookTo()), zoom);
    setLookFrom(getLookFrom() - glm::normalize(direction) * static_cast<float>(zoom));
}

void Trackball::stepPan(Direction dir) {
    vec2 origin = vec2(0.5, 0.5);
    vec2 destination = origin;

    switch (dir) {
        case Direction::Up:
            destination.y -= stepsize;
            break;

        case Direction::Left:
            destination.x -= stepsize;
            break;

        case Direction::Down:
            destination.y += stepsize;
            break;

        case Direction::Right:
            destination.x += stepsize;
            break;
    }
    if (!allowHorizontalPanning_) destination.x = origin.x;
    if (!allowVerticalPanning_) destination.y = origin.y;

    vec3 fromNormalizedDeviceCoord(
        object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(origin));
    vec3 toNormalizedDeviceCoord(2.f * destination - 1.f, fromNormalizedDeviceCoord.z);
    dvec3 translation(
        getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord));
    vec3 boundedTranslation(
        getBoundedTranslation(dvec3(getLookFrom()), dvec3(getLookTo()), translation));
    setLook(getLookFrom() - boundedTranslation, getLookTo() - boundedTranslation, getLookUp());
}

void Trackball::rotateTrackBall(const vec3& fromTrackBallPos, const vec3& toTrackBallPos) {
    vec3 view = glm::normalize(getLookFrom() - getLookTo());
    vec3 right = glm::cross(getLookUp(), view);
    // Transform virtual sphere coordinates to view space
    vec3 Pa =
        fromTrackBallPos.x * right + fromTrackBallPos.y * getLookUp() + fromTrackBallPos.z * view;
    vec3 Pc = toTrackBallPos.x * right + toTrackBallPos.y * getLookUp() + toTrackBallPos.z * view;
    // Compute the rotation that transforms coordinates
    glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
    setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(),
            glm::rotate(quaternion, getLookUp()));
}

dvec3 Trackball::getBoundedTranslation(const dvec3& lookFrom, const dvec3& lookTo,
                                       dvec3 translation) {
    // Make sure that we do not translate outside of the specified boundaries

    // To avoid sliding motions along boundaries created by clamping we
    // simply disallow movements that would cause the lookTo point to
    // go out of bounds
    auto newPos(lookTo - translation);
    // auto clampedPos = glm::clamp(newPos, dvec3(getLookToMinValue()), dvec3(getLookToMaxValue()));
    auto distanceToMinBounds = newPos - dvec3(getLookToMinValue());
    auto distanceToMaxBounds = dvec3(getLookToMaxValue()) - newPos;
    auto axesMinDistance = glm::min(distanceToMinBounds, distanceToMaxBounds);
    auto minDistance = glm::min(glm::min(axesMinDistance.y, axesMinDistance.z), axesMinDistance.x);
    // Negative distance means that we would move outside of boundaries
    if (minDistance < 0) {
        translation = dvec3(0);
    }
    // Clamping does not work when movement is restricted along horizontal or vertical axes.
    // else {
    //
    //    translation = glm::clamp(translation, dvec3(-axesMinDistance), dvec3(axesMinDistance));
    //}
    // minDistance = minDistance < 0 ? 0 : minDistance;
    // translation = glm::clamp(translation, dvec3(-minDistance), dvec3(minDistance));

    // translation = lookTo - glm::clamp(lookTo - translation, dvec3(getLookToMinValue()),
    // dvec3(getLookToMaxValue()));

    // Currently we cannot enforce the lookFrom boundaries since the zooming does not respect
    // the lookFrom boundaries. This would create a jerky motion when clamping lookFrom after
    // zooming.
    // TODO: Change this when better lookFrom boundaries have been set.
    // translation = lookFrom - glm::clamp(lookFrom - translation, dvec3(getLookFromMinValue()),
    // dvec3(getLookFromMaxValue()));

    // Ensures that the user can rotate around the look to point.
    // This should be resolved using smaller lookTo bounds instead!
    // auto radius = glm::length(lookFrom - lookTo);
    // translation = lookTo - glm::clamp(lookTo - translation, dvec3(getLookFromMinValue()) +
    // radius, dvec3(getLookFromMaxValue()) - radius);
    return translation;
}

double inviwo::Trackball::getBoundedZoom(const dvec3& lookFrom, const dvec3& zoomTo, double zoom) {
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
        auto lookToBounds = 0.5 * (dvec3(getLookToMaxValue()) - dvec3(getLookToMinValue()));
        maxZoomOut =
            directionLength - glm::min(glm::min(lookToBounds.y, lookToBounds.z), lookToBounds.x);
    } else {
        maxZoomOut =
            directionLength - glm::min(glm::min(minDistance.y, minDistance.z), minDistance.x);
    }

    // Clamp so that the user does not zoom outside of the bounds and not
    // further than, or onto, the lookTo point.
    zoom = glm::clamp(zoom, maxZoomOut,
                      directionLength - std::max(maxZoomInDistance_.get(),
                                                 static_cast<double>(object_->getNearPlaneDist())));
    return zoom;
}

void Trackball::rotateLeft(Event* event) {
    stepRotate(Direction::Left);
    event->markAsUsed();
}

void Trackball::rotateRight(Event* event) {
    stepRotate(Direction::Right);
    event->markAsUsed();
}

void Trackball::rotateUp(Event* event) {
    stepRotate(Direction::Up);
    event->markAsUsed();
}

void Trackball::rotateDown(Event* event) {
    stepRotate(Direction::Down);
    event->markAsUsed();
}

void Trackball::panLeft(Event* event) {
    stepPan(Direction::Left);
    event->markAsUsed();
}

void Trackball::panRight(Event* event) {
    stepPan(Direction::Right);
    event->markAsUsed();
}

void Trackball::panUp(Event* event) {
    stepPan(Direction::Up);
    event->markAsUsed();
}

void Trackball::panDown(Event* event) {
    stepPan(Direction::Down);
    event->markAsUsed();
}

void Trackball::zoomIn(Event* event) {
    stepZoom(Direction::Up);
    event->markAsUsed();
}

void Trackball::zoomOut(Event* event) {
    stepZoom(Direction::Down);
    event->markAsUsed();
}

void Trackball::reset(Event* event) {
    isMouseBeingPressedAndHold_ = false;
    if (animate_) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() -
                                                                  lastRotTime_)
                .count() < 100) {
            timer_.start();
        }
    }
    event->markAsUsed();
}

const vec3& Trackball::getLookFrom() const { return object_->getLookFrom(); }

const vec3& Trackball::getLookUp() const { return object_->getLookUp(); }

const vec3 Trackball::getLookFromMinValue() const { return object_->getLookFromMinValue(); }

const vec3 Trackball::getLookFromMaxValue() const { return object_->getLookFromMaxValue(); }

const vec3 Trackball::getLookToMinValue() const { return object_->getLookToMinValue(); }

const vec3 Trackball::getLookToMaxValue() const { return object_->getLookToMaxValue(); }

void Trackball::setLookTo(vec3 lookTo) { object_->setLookTo(lookTo); }

void Trackball::setLookFrom(vec3 lookFrom) { object_->setLookFrom(lookFrom); }

void Trackball::setLookUp(vec3 lookUp) { object_->setLookUp(lookUp); }

void Trackball::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    object_->setLook(lookFrom, lookTo, lookUp);
}

void inviwo::Trackball::animate() {
    if (this->evaluated_) {
        this->evaluated_ = false;
        dispatchFront([this]() {
            const glm::quat identity(1.0f, 0.0f, 0.0f, 0.0f);
            const float t = 0.1f;
            const float dot = glm::dot(lastRot_, identity);
            const float theta = std::acos(dot);
            const float sintheta = std::sin(theta);
            lastRot_ = lastRot_ * (std::sin((1.0f - t) * theta) / sintheta) +
                       identity * (std::sin(t * theta) / sintheta);
            setLook(getLookTo() + glm::rotate(lastRot_, getLookFrom() - getLookTo()), getLookTo(),
                    glm::rotate(lastRot_, getLookUp()));

            if ((lastRot_.x - identity.x) * (lastRot_.x - identity.x) +
                    (lastRot_.y - identity.y) * (lastRot_.y - identity.y) +
                    (lastRot_.z - identity.z) * (lastRot_.z - identity.z) +
                    (lastRot_.w - identity.w) * (lastRot_.w - identity.w) <
                0.000001f)
                timer_.stop();
            this->evaluated_ = true;
        });
    }
}

}  // namespace
