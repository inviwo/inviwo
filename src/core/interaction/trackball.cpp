/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/trackballobject.h>
#include <inviwo/core/util/intersection/raysphereintersection.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>


namespace inviwo {

Trackball::Trackball(TrackballObject* object)
    : CompositeProperty("trackball", "Trackball")
    , object_(object)
    , isMouseBeingPressedAndHold_(false)
    , lastNDC_(vec3(0.0))
    , gestureStartNDCDepth_(-1)
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true,
                               InvalidationLevel::Valid)

    , allowHorizontalPanning_("allowHorizontalPanning", "Horizontal panning enabled", true)
    , allowVerticalPanning_("allowVerticalPanning", "Vertical panning enabled", true)
    , allowZooming_("allowZoom", "Zoom enabled", true)
    , maxZoomInDistance_("minDistanceToLookAtPoint", "Minimum zoom distance", 0.0f, 0.0f, 1000.0f)

    , allowHorizontalRotation_("allowHorziontalRotation", "Rotation around horizontal axis", true)
    , allowVerticalRotation_("allowVerticalRotation", "Rotation around vertical axis", true)
    , allowViewDirectionRotation_("allowViewAxisRotation", "Rotation around view axis", true)
    , allowRecenterView_("allowRecenterView", "Recenter view with Double Click", false)
    , animate_("animate", "Animate rotations", false)

    , mouseRotate_("trackballRotate", "Rotate", [this](Event* e) { rotate(e); }, MouseButton::Left,
                   MouseState::Press | MouseState::Move)

    , mouseZoom_("trackballZoom", "Zoom", [this](Event* e) { zoom(e); }, MouseButton::Right,
                 MouseState::Press | MouseState::Move)

    , mousePan_("trackballPan", "Pan", [this](Event* e) { pan(e); }, MouseButton::Middle,
                MouseState::Press | MouseState::Move)

    , mouseRecenterFocusPoint_("mouseRecenterFocusPoint", "Recenter Focus Point",
                             [this](Event* e) { recenterFocusPoint(e); }, MouseButton::Left,
                             MouseState::DoubleClick)

    , mouseReset_("mouseReset", "Reset", [this](Event* e) { reset(e); }, MouseButtons(flags::any),
                  MouseState::Release)

    , stepRotateUp_("stepRotateUp", "Rotate up", [this](Event* e) { rotateUp(e); }, IvwKey::W,
                    KeyState::Press)

    , stepRotateLeft_("stepRotateLeft", "Rotate left", [this](Event* e) { rotateLeft(e); },
                      IvwKey::A, KeyState::Press)

    , stepRotateDown_("stepRotateDown", "Rotate down", [this](Event* e) { rotateDown(e); },
                      IvwKey::S, KeyState::Press)

    , stepRotateRight_("stepRotateRight", "Rotate right", [this](Event* e) { rotateRight(e); },
                       IvwKey::D, KeyState::Press)

    , stepZoomIn_("stepZoomIn", "Zoom in", [this](Event* e) { zoomIn(e); }, IvwKey::R,
                  KeyState::Press)

    , stepZoomOut_("stepZoomOut", "Zoom out", [this](Event* e) { zoomOut(e); }, IvwKey::F,
                   KeyState::Press)

    , stepPanUp_("stepPanUp", "Pan up", [this](Event* e) { panUp(e); }, IvwKey::W, KeyState::Press,
                 KeyModifier::Shift)

    , stepPanLeft_("stepPanLeft", "Pan left", [this](Event* e) { panLeft(e); }, IvwKey::A,
                   KeyState::Press, KeyModifier::Shift)

    , stepPanDown_("stepPanDown", "Pan down", [this](Event* e) { panDown(e); }, IvwKey::S,
                   KeyState::Press, KeyModifier::Shift)

    , stepPanRight_("stepPanRight", "Pan right", [this](Event* e) { panRight(e); }, IvwKey::D,
                    KeyState::Press, KeyModifier::Shift)

    , touchGesture_("touchGesture", "Touch", [this](Event* e) { touchGesture(e); },
                    util::make_unique<GeneralEventMatcher>(
                        [](Event* e) { return e->hash() == TouchEvent::chash(); }))
    
    , evaluated_(true)
    , timer_(30, [this]() { animate(); }) 
    , followObjectDuringRotation_(true)
{
    
    mouseReset_.setVisible(false);
    mouseReset_.setCurrentStateAsDefault();

    addProperty(handleInteractionEvents_);

    addProperty(allowHorizontalPanning_);
    addProperty(allowVerticalPanning_);
    addProperty(allowZooming_);
    //addProperty(maxZoomInDistance_);

    addProperty(allowHorizontalRotation_);
    addProperty(allowVerticalRotation_);
    addProperty(allowViewDirectionRotation_);
    addProperty(allowRecenterView_);

    addProperty(animate_);

    addProperty(mouseRotate_);
    addProperty(mouseZoom_);
    addProperty(mousePan_);
    addProperty(mouseRecenterFocusPoint_);
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

    auto systemSettings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    followObjectDuringRotation_ = systemSettings->followObjectDuringRotation_.get();
    systemSettings->followObjectDuringRotation_.onChange([systemSettings,this]() {
        followObjectDuringRotation_ = systemSettings->followObjectDuringRotation_.get();
    });
}

Trackball::Trackball(const Trackball& rhs)
    : CompositeProperty(rhs)
    , object_(rhs.object_)
    , isMouseBeingPressedAndHold_(false)
    , lastNDC_(vec3(0.0))
    , gestureStartNDCDepth_(-1)
    , handleInteractionEvents_(rhs.handleInteractionEvents_)
    , allowHorizontalPanning_(rhs.allowHorizontalPanning_)
    , allowVerticalPanning_(rhs.allowVerticalPanning_)
    , allowZooming_(rhs.allowZooming_)
    , maxZoomInDistance_(rhs.maxZoomInDistance_)
    , allowHorizontalRotation_(rhs.allowHorizontalRotation_)
    , allowVerticalRotation_(rhs.allowVerticalRotation_)
    , allowViewDirectionRotation_(rhs.allowViewDirectionRotation_)
    , allowRecenterView_(rhs.allowRecenterView_)
    , animate_(rhs.animate_)
    , mouseRotate_(rhs.mouseRotate_)
    , mouseZoom_(rhs.mouseZoom_)
    , mousePan_(rhs.mousePan_)
    , mouseRecenterFocusPoint_(rhs.mouseRecenterFocusPoint_)
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
    , timer_(30, [this]() { animate(); }) 
    , followObjectDuringRotation_(rhs.followObjectDuringRotation_)
{

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
    addProperty(allowRecenterView_);

    addProperty(animate_);

    addProperty(mouseRotate_);
    addProperty(mouseZoom_);
    addProperty(mousePan_);
    addProperty(mouseRecenterFocusPoint_);
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


    auto systemSettings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    followObjectDuringRotation_ = systemSettings->followObjectDuringRotation_.get();
    systemSettings->followObjectDuringRotation_.onChange( [&](){
        followObjectDuringRotation_ = systemSettings->followObjectDuringRotation_.get();
    });

}

Trackball& Trackball::operator=(const Trackball& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        object_ = that.object_;
        isMouseBeingPressedAndHold_ = false;
        lastNDC_ = vec3(0.0);
        gestureStartNDCDepth_ = -1;
        handleInteractionEvents_ = that.handleInteractionEvents_;
        allowHorizontalPanning_ = that.allowHorizontalPanning_;
        allowVerticalPanning_ = that.allowVerticalPanning_;
        allowZooming_ = that.allowZooming_;
        allowHorizontalRotation_ = that.allowHorizontalRotation_;
        allowVerticalRotation_ = that.allowVerticalRotation_;
        allowViewDirectionRotation_ = that.allowViewDirectionRotation_;
        allowRecenterView_ = that.allowRecenterView_;
        animate_ = that.animate_;
        mouseRotate_ = that.mouseRotate_;
        mouseZoom_ = that.mouseZoom_;
        mousePan_ = that.mousePan_;
        mouseRecenterFocusPoint_ = that.mouseRecenterFocusPoint_;
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
        followObjectDuringRotation_ = that.followObjectDuringRotation_;
    }
    return *this;
}

Trackball::~Trackball() = default;

void Trackball::invokeEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    CompositeProperty::invokeEvent(event);
}

const vec3& Trackball::getLookTo() const { return object_->getLookTo(); }

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

vec3 Trackball::mapNormalizedMousePosToTrackball(const vec2& mousePos, float r) {
    // set x and y to lie in interval [-r, r]
    const vec2 centerOffset = vec2(2.f * mousePos.x - 1.f, 2.f * (1.f - mousePos.y) - 1.f);
    const float norm = glm::length2(centerOffset);
    float z = 0;
    // Mapping according to Holroyds trackball
    // Piece-wise sphere + hyperbolic sheet
    if (norm <= r * r / (2.0f)) {  // Spherical Region
        z = r * r - norm;
        z = z > 0.0f ? sqrtf(z) : 0.0f;
    } else {  // Hyperbolic Region - for smooth z values
        z = ((r * r) / (2.0f * sqrtf(norm)));
    }

    return vec3(centerOffset.x, centerOffset.y, z);
}

vec3 Trackball::getWorldSpaceTranslationFromNDCSpace(const vec3& fromNDC,
                                                     const vec3& toNDC) {
    const auto prevWorldPos = object_->getWorldPosFromNormalizedDeviceCoords(vec3(fromNDC));
    const auto worldPos = object_->getWorldPosFromNormalizedDeviceCoords(toNDC);
    return worldPos - prevWorldPos;
}

std::pair<bool, vec3> Trackball::getTrackBallIntersection(const vec2 pos) const {
    const auto rayOrigin = object_->getWorldPosFromNormalizedDeviceCoords(vec3(pos.x, pos.y, -1.f));
    const auto direction = glm::normalize(
        object_->getWorldPosFromNormalizedDeviceCoords(vec3(pos.x, pos.y, 0.f)) - rayOrigin);
    const auto res = raySphereIntersection(getLookTo(), trackBallWorldSpaceRadius_, rayOrigin,
                                           direction, 0.0f, std::numeric_limits<float>::max());
    return {res.first, rayOrigin + direction * res.second};
}

void Trackball::rotate(Event* event) {
    if (!allowHorizontalRotation_ && !allowVerticalRotation_) return;
    timer_.stop();

    auto mouseEvent = static_cast<MouseEvent*>(event);
    const auto ndc = static_cast<vec3>(mouseEvent->ndc());
    
    const auto curNDC =
        vec3(allowHorizontalRotation_ ? ndc.x : 0.0f, allowVerticalRotation_ ? ndc.y : 0.0f,followObjectDuringRotation_ ?  ndc.z : 1);

    const auto& to = getLookTo();
    const auto& from = getLookFrom();
    const auto& up = getLookUp();

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        isMouseBeingPressedAndHold_ = true;
        gestureStartNDCDepth_ = curNDC.z;
        trackBallWorldSpaceRadius_ =
            glm::distance(to, object_->getWorldPosFromNormalizedDeviceCoords(curNDC));
    } else {
        // Compute coordinates on a sphere to rotate from and to
        const auto lastTBI = getTrackBallIntersection(lastNDC_.xy());
        const auto curTBI = getTrackBallIntersection(curNDC.xy());

        if (lastTBI.first && curTBI.first && gestureStartNDCDepth_ < 1) {
            const auto Pa = glm::normalize(lastTBI.second - to);
            const auto Pc = glm::normalize(curTBI.second - to);
            lastRot_ = glm::quat(Pc, Pa);
        } else {
            const auto rot = glm::half_pi<float>() * (curNDC - lastNDC_);
            const auto Pa = glm::normalize(from - to);
            const auto Pc = glm::rotate(glm::rotate(Pa, rot.y, glm::cross(Pa, up)), rot.x, up);
            lastRot_ = glm::quat(Pc, Pa);
        }
        lastRotTime_ = std::chrono::system_clock::now();
        setLook(getLookTo() + glm::rotate(lastRot_, from - to), to, glm::rotate(lastRot_, up));
    }
    // update mouse positions
    lastNDC_ = curNDC;
    event->markAsUsed();
}

void Trackball::zoom(Event* event) {
    if (!allowZooming_) return;
    timer_.stop();

    auto mouseEvent = static_cast<MouseEvent*>(event);
    auto curNDC = static_cast<vec3>(mouseEvent->ndc());

    const auto& to = getLookTo();
    const auto& from = getLookFrom();

    // compute direction vector
    const vec3 direction = from - to;
    const float directionLength = glm::length(direction);
    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        isMouseBeingPressedAndHold_ = true;
    } else if (curNDC.y != lastNDC_.y && directionLength > 0) {
        // use the difference in mouse y-position to determine amount of zoom
        const auto zoom = (curNDC.y - lastNDC_.y) * directionLength;
        const auto boundedZoom = getBoundedZoom(from, to, zoom);
        // zoom by moving the camera
        setLookFrom(from - glm::normalize(direction) * boundedZoom);
    }

    lastNDC_ = curNDC;
    event->markAsUsed();
}

void Trackball::pan(Event* event) {
    if (!allowHorizontalPanning_ && !allowVerticalPanning_) return;

    timer_.stop();
    auto mouseEvent = static_cast<MouseEvent*>(event);
    auto curNDC = static_cast<vec3>(mouseEvent->ndc());

    // disable movements on first press
    if (!isMouseBeingPressedAndHold_) {
        isMouseBeingPressedAndHold_ = true;
        if (curNDC.z >= 1.0) {
            curNDC.z = object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
                                  vec2(curNDC.x, curNDC.y))
                           .z;
        }
        gestureStartNDCDepth_ = curNDC.z;

    } else {
        const auto fromNDC = vec3(lastNDC_.x, lastNDC_.y, gestureStartNDCDepth_);
        const auto toNDC =
            vec3(allowHorizontalPanning_ ? curNDC.x : fromNDC.x,
                 allowVerticalPanning_ ? curNDC.y : fromNDC.y, gestureStartNDCDepth_);

        const auto& to = getLookTo();
        const auto& from = getLookFrom();
        const auto& up = getLookUp();

        const auto translation = getWorldSpaceTranslationFromNDCSpace(fromNDC, toNDC);
        const auto boundedTranslation = getBoundedTranslation(from, to, translation);

        setLook(from - boundedTranslation, to - boundedTranslation, up);
    }

    lastNDC_ = curNDC;

    event->markAsUsed();
}

void Trackball::reset(Event* event) {
    if (isMouseBeingPressedAndHold_) {
        isMouseBeingPressedAndHold_ = false;
        if (animate_ &&
            std::chrono::system_clock::now() - lastRotTime_ < std::chrono::milliseconds(100)) {
            timer_.start();
        }
        event->markAsUsed();
    }
}

void Trackball::stepRotate(Direction dir) {
    const vec2 origin = vec2(0.5, 0.5);
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

    const auto trackballDirection = mapNormalizedMousePosToTrackball(direction);
    const auto trackballOrigin = mapNormalizedMousePosToTrackball(origin);
    rotateTrackBall(trackballOrigin, trackballDirection);
}

void Trackball::stepZoom(Direction dir) {
    if (!allowZooming_) return;
   
    // compute direction vector
    const auto direction = getLookFrom() - getLookTo();
    const auto directionLength = glm::length(direction);
    auto zoom = 0.0f;
    if (dir == Direction::Up) {
        zoom = stepsize * directionLength;
    } else if (dir == Direction::Down) {
        zoom = -stepsize * directionLength;
    }

    // zoom by moving the camera
    const auto boundedZoom = getBoundedZoom(getLookFrom(), getLookTo(), zoom);
    setLookFrom(getLookFrom() - glm::normalize(direction) * boundedZoom);
}

void Trackball::stepPan(Direction dir) {
    const vec2 origin = vec2(0.5, 0.5);
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

    const vec3 fromNormalizedDeviceCoord(
        object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(origin));
    const vec3 toNormalizedDeviceCoord(2.f * destination - 1.f, fromNormalizedDeviceCoord.z);
    const vec3 translation(
        getWorldSpaceTranslationFromNDCSpace(fromNormalizedDeviceCoord, toNormalizedDeviceCoord));
    const vec3 boundedTranslation(getBoundedTranslation(getLookFrom(), getLookTo(), translation));
    setLook(getLookFrom() - boundedTranslation, getLookTo() - boundedTranslation, getLookUp());
}

void Trackball::touchGesture(Event* event) {
    TouchEvent* touchEvent = static_cast<TouchEvent*>(event);

    // Use the two closest points to extract translation, scaling and rotation
    const auto& touchPoints = touchEvent->touchPoints();

    if (touchPoints.size() == 1) {
        const auto& point = touchPoints[0];
        if (point.state() == TouchState::Finished) return reset(event);

        if (!allowHorizontalRotation_ && !allowVerticalRotation_) return;
        timer_.stop();

        const auto ndc = static_cast<vec3>(point.ndc());
        const auto curNDC = vec3(allowHorizontalRotation_ ? ndc.x : 0.0f,
                                 allowVerticalRotation_ ? ndc.y : 0.0f, ndc.z);

        const auto& to = getLookTo();
        const auto& from = getLookFrom();
        const auto& up = getLookUp();

        // disable movements on first press
        if (!isMouseBeingPressedAndHold_) {
            isMouseBeingPressedAndHold_ = true;
            gestureStartNDCDepth_ = curNDC.z;
            trackBallWorldSpaceRadius_ =
                glm::distance(to, object_->getWorldPosFromNormalizedDeviceCoords(curNDC));
        } else {
            // Compute coordinates on a sphere to rotate from and to
            const auto lastTBI = getTrackBallIntersection(lastNDC_.xy());
            const auto curTBI = getTrackBallIntersection(curNDC.xy());

            if (lastTBI.first && curTBI.first && gestureStartNDCDepth_ < 1) {
                const auto Pa = glm::normalize(lastTBI.second - to);
                const auto Pc = glm::normalize(curTBI.second - to);
                lastRot_ = glm::quat(Pc, Pa);
            } else {
                const auto rot = glm::half_pi<float>() * (curNDC - lastNDC_);
                const auto Pa = glm::normalize(from - to);
                const auto Pc = glm::rotate(glm::rotate(Pa, rot.y, glm::cross(Pa, up)), rot.x, up);
                lastRot_ = glm::quat(Pc, Pa);
            }
            lastRotTime_ = std::chrono::system_clock::now();
            setLook(getLookTo() + glm::rotate(lastRot_, from - to), to, glm::rotate(lastRot_, up));
        }
        // update mouse positions
        lastNDC_ = curNDC;
        event->markAsUsed();

    } else if (touchPoints.size() > 1) {
        const auto& touchPoint1 = touchPoints[0];
        const auto& touchPoint2 = touchPoints[1];

        const auto prevPos1 = touchPoint1.prevPosNormalized();
        const auto prevPos2 = touchPoint2.prevPosNormalized();
        const auto pos1 = touchPoint1.posNormalized();
        const auto pos2 = touchPoint2.posNormalized();

        auto v1(prevPos2 - prevPos1);
        auto v2(pos2 - pos1);

        // The glm implementation of orientedAngle returns positive angle
        // for small negative angles.
        // auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
        // Roll our own angle calculation
        auto v1Normalized = glm::normalize(v1);
        auto v2Normalized = glm::normalize(v2);
        auto angle = acos(glm::clamp(glm::dot(v1Normalized, v2Normalized), -1.0, 1.0));
        // Check orientation using cross product.
        if (v1Normalized.x * v2Normalized.y - v2Normalized.x * v1Normalized.y < 0) {
            angle = -angle;
        }

        auto zoom = 1 - glm::length(v1) / glm::length(v2);
        if (!std::isfinite(zoom) || !allowZooming_) {
            zoom = 0;
        }
        // Difference between midpoints before and after
        const auto prevCenterPoint = glm::mix(prevPos1, prevPos2, 0.5);
        const auto centerPoint = glm::mix(pos1, pos2, 0.5);

        if (touchPoint1.state() == TouchState::Stationary ||
            touchPoint2.state() == TouchState::Stationary) {
            gestureStartNDCDepth_ = std::min(touchPoint1.depth(), touchPoint2.depth());
            if (gestureStartNDCDepth_ >= 1.) {
                gestureStartNDCDepth_ =
                    object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(pos1).z;
            }
        }

        // Compute translation in world space
        vec3 fromNDC(2. * prevCenterPoint - 1., gestureStartNDCDepth_);
        vec3 toNDC(2. * centerPoint - 1., gestureStartNDCDepth_);
        if (!allowHorizontalPanning_) toNDC.x = fromNDC.x;
        if (!allowVerticalPanning_) toNDC.y = fromNDC.y;

        const vec3 worldSpaceTranslation(getWorldSpaceTranslationFromNDCSpace(fromNDC, toNDC));

        // Zoom based on the closest point to the object
        // Use the look at point if the closest point is unknown
        auto depth = std::min(touchPoint1.depth(), touchPoint2.depth());
        if (depth <= -1 || depth >= 1) {
            // Get NDC depth of the lookTo position
            depth = object_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(vec2(0.f)).z;
        }
        const auto zoomToWorldPos(object_->getWorldPosFromNormalizedDeviceCoords(vec3(0.f, 0.f, depth)));
        auto direction = zoomToWorldPos - getLookFrom();
        zoom *= glm::length(direction);
        direction = glm::normalize(direction);
        zoom = getBoundedZoom(getLookFrom(), getLookTo(), static_cast<float>(zoom));
        const vec3 newLookFrom = getLookFrom() + static_cast<float>(zoom) * (direction);

        vec3 boundedWorldSpaceTranslation(
            getBoundedTranslation(newLookFrom, getLookTo(), worldSpaceTranslation));
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

        isMouseBeingPressedAndHold_ = false;
        touchEvent->markAsUsed();
    }
}

void Trackball::rotateTrackBall(const vec3& fromTrackBallPos, const vec3& toTrackBallPos) {
    const vec3 view = glm::normalize(getLookFrom() - getLookTo());
    const vec3 right = glm::cross(getLookUp(), view);
    // Transform virtual sphere coordinates to view space
    const vec3 Pa =
        fromTrackBallPos.x * right + fromTrackBallPos.y * getLookUp() + fromTrackBallPos.z * view;
    const vec3 Pc = toTrackBallPos.x * right + toTrackBallPos.y * getLookUp() + toTrackBallPos.z * view;
    // Compute the rotation that transforms coordinates
    const glm::quat quaternion = glm::quat(glm::normalize(Pc), glm::normalize(Pa));
    setLook(getLookTo() + glm::rotate(quaternion, getLookFrom() - getLookTo()), getLookTo(),
            glm::rotate(quaternion, getLookUp()));
}

vec3 Trackball::getBoundedTranslation(const vec3& lookFrom, const vec3& lookTo, vec3 translation) {
    // Make sure that we do not translate outside of the specified boundaries

    // To avoid sliding motions along boundaries created by clamping we
    // simply disallow movements that would cause the lookTo point to
    // go out of bounds
    const auto newPos(lookTo - translation);
    // auto clampedPos = glm::clamp(newPos, dvec3(getLookToMinValue()), dvec3(getLookToMaxValue()));
    const auto distanceToMinBounds = newPos - getLookToMinValue();
    const auto distanceToMaxBounds = getLookToMaxValue() - newPos;
    const auto axesMinDistance = glm::min(distanceToMinBounds, distanceToMaxBounds);
    const auto minDistance =
        glm::min(glm::min(axesMinDistance.y, axesMinDistance.z), axesMinDistance.x);
    // Negative distance means that we would move outside of boundaries
    if (minDistance < 0) {
        translation = vec3(0);
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

float inviwo::Trackball::getBoundedZoom(const vec3& lookFrom, const vec3& zoomTo, float zoom) {
    // Compute the smallest distance between the bounds of lookTo and lookFrom
    const auto distanceToMinBounds = glm::abs(getLookFromMinValue() - getLookToMinValue());
    const auto distanceToMaxBounds = glm::abs(getLookFromMaxValue() - getLookToMaxValue());
    const auto minDistance = glm::min(distanceToMinBounds, distanceToMaxBounds);
    const auto directionLength = glm::length(lookFrom - zoomTo);

    // Use a heuristic if the lookFrom and lookTo bounds are equal:
    // One cannot zoom out more than half of the smallest bound in xyz.
    // Otherwise:
    // The distance between the lookFrom and lookTo cannot be greater
    // than the smallest distance between the two bounds,
    // thereby ensuring that the lookFrom and lookTo are inside their bounds.

    // If the bounds of lookFrom and lookTo are equal.
    // Using half of the smallest axis will NOT ensure that lookFrom stays
    // within the bounds but is best for backwards compatibility
    // (distance between lookFrom and lookTo will be too small otherwise)

    const auto maxZoomOut =
        glm::any(glm::equal(minDistance, vec3(0)))
            ? directionLength - 0.5f * glm::compMin(getLookToMaxValue() - getLookToMinValue())
            : directionLength - glm::compMin(minDistance);

    // Clamp so that the user does not zoom outside of the bounds and not
    // further than, or onto, the lookTo point.
    return glm::clamp(zoom, maxZoomOut, directionLength - std::max(maxZoomInDistance_.get(),
                                                                   object_->getNearPlaneDist()));
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

void Trackball::recenterFocusPoint(Event *event) {
    if (!allowRecenterView_.get()) {
        return;
    }

    if (auto mouseEvent = dynamic_cast<MouseEvent*>(event)) {
        auto p = mouseEvent->ndc();

        if (std::abs(p.z - 1.0) < glm::epsilon<decltype(p.z)>()) return;

        auto newLookTo = object_->getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(p));
        auto newLookFrom = getLookFrom() + (newLookTo - getLookTo());

        setLookTo(newLookTo);
        setLookFrom(newLookFrom);
    }
    event->markAsUsed();
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
