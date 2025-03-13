/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/core/util/timer.h>
#include <inviwo/core/util/glm.h>

#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/epsilon.hpp>

namespace inviwo {

class Event;
class TrackballObject;

class IVW_CORE_API Trackball : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.Trackball"};

    /**
     * Rotates and moves object around a sphere.
     * The trackball does not take ownership of pointers handed to it.
     * @see CameraTrackball
     */
    Trackball(TrackballObject* object);

    Trackball(std::string_view identifier, std::string_view displayName,
              TrackballObject* object = nullptr);

    Trackball(const Trackball& rhs);
    virtual Trackball* clone() const override;
    virtual ~Trackball();

    TrackballObject* getTrackballObject() const;
    void setTrackballObject(TrackballObject* obj);

    virtual void invokeEvent(Event* event) override;

    const vec3 getLookTo() const;
    const vec3 getLookFrom() const;
    const vec3 getLookUp() const;
    const vec3 getLookRight() const;

    const vec3 getLookFromMinValue() const;
    const vec3 getLookFromMaxValue() const;

    const vec3 getLookToMinValue() const;
    const vec3 getLookToMaxValue() const;

    void setLookTo(vec3 lookTo);
    void setLookFrom(vec3 lookFrom);
    void setLookUp(vec3 lookUp);

    /**
     * \brief Set look from, look to and up vector at the same time.
     * Should be used when more than one parameter will be changed to avoid duplicate evaluations.
     *
     * @param lookFrom
     * @param lookTo
     * @param lookUp
     */
    void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp);

    vec3 getWorldSpaceTranslationFromNDCSpace(const vec3& fromNormalizedDeviceCoord,
                                              const vec3& toNormalizedDeviceCoord);

protected:
    enum class Direction { Up = 0, Down, Left, Right };

    vec3 mapNormalizedMousePosToTrackball(const vec2& mousePos, float radius = 1.0f);
    void rotateTrackBall(const vec3& fromTrackballPos, const vec3& toTrackballPos);
    vec3 getBoundedTranslation(const vec3& lookFrom, const vec3& lookTo, vec3 translation);
    float getBoundedZoom(const vec3& lookFrom, const vec3& zoomTo, float zoom);
    std::pair<bool, vec3> getTrackBallIntersection(const vec2 pos) const;

    void rotate(Event* event);
    void rotateTAV(Event* event);
    void rotateArc(Event* event, bool followObjectDuringRotation = false);
    void rotateFPS(Event* event);
    void zoom(Event* event);
    void pan(Event* event);
    void reset(Event* event);

    void moveLeft(Event* event);
    void moveRight(Event* event);
    void moveUp(Event* event);
    void moveDown(Event* event);
    void moveForward(Event* event);
    void moveBackward(Event* event);

    const vec3 getWorldUp() const;
    mat4 roll(const float radians) const;
    mat4 pitch(const float radians) const;
    mat4 yaw(const float radians) const;

    void stepRotate(Direction dir);
    void stepZoom(Direction dir, const int numSteps = 1);
    void stepPan(Direction dir);

    void rotateLeft(Event* event);
    void rotateRight(Event* event);
    void rotateUp(Event* event);
    void rotateDown(Event* event);

    void panLeft(Event* event);
    void panRight(Event* event);
    void panUp(Event* event);
    void panDown(Event* event);

    void zoomWheel(Event* event);
    void zoomIn(Event* event, const int numSteps = 1);
    void zoomOut(Event* event, const int numSteps = 1);

    void recenterFocusPoint(Event* event);

    /**
     * \brief Rotates around the direction vector,
     * zooms along the direction vector and translates along up/right vector.
     *
     * @param event TouchEvent
     */
    void touchGesture(Event* event);
    void animate();

    TrackballObject* object_;
    bool isMouseBeingPressedAndHold_;

    vec3 lastNDC_;

    double gestureStartNDCDepth_;
    float trackBallWorldSpaceRadius_;

    static constexpr float radius = 0.5f;  ///< Radius in normalized screen space [0 1]^2
    static constexpr float stepsize = 0.05f;

    glm::quat lastRot_;
    std::chrono::system_clock::time_point lastRotTime_;
    bool evaluated_;
    Timer timer_;

public:
    OptionPropertyInt trackballMethod_;  /// Chooses which trackball method to use (mouse only,
                                         /// touch always follows finger)
    FloatProperty sensitivity_;          /// Controls the rotation sensitivity
    FloatProperty movementSpeed_;
    BoolProperty fixUp_;                /// Fixes the up vector to world_up in all rotation methods
    OptionPropertyInt worldUp_;         /// Defines which axis is considered up in world space
    FloatVec3Property customWorldUp_;   /// The custom world up direction (normalized)
    FloatProperty verticalAngleLimit_;  /// Limits the angle between world up and view direction
                                        /// when fixUp is True

    // Interaction restrictions
    BoolProperty handleInteractionEvents_;
    // Options to restrict translation along view-space axes.
    BoolProperty allowHorizontalPanning_;  ///< Enable/disable horizontal panning
    BoolProperty allowVerticalPanning_;    ///< Enable/disable vertical panning
    BoolProperty boundedPanning_;
    BoolProperty allowZooming_;       ///< Enable/disable zooming
    BoolProperty allowWheelZooming_;  ///< Enable/disable zooming using the mouse wheel
    BoolProperty boundedZooming_;

    // Options to restrict rotation around view-space axes.
    BoolProperty allowHorizontalRotation_;  ///< Enable/disable rotation around horizontal axis
    BoolProperty allowVerticalRotation_;    ///< Enable/disable rotation around vertical axis

    // Enable/disable rotation around view direction axis
    BoolProperty allowViewDirectionRotation_;

    BoolProperty allowRecenterView_;  ///< recenter the camera focus point with a double click

    BoolProperty animate_;

    // Event Properties.
    EventProperty mouseRecenterFocusPoint_;
    EventProperty wheelZoom_;

    EventProperty mouseRotate_;
    EventProperty mouseZoom_;
    EventProperty mousePan_;
    EventProperty mouseReset_;

    EventProperty moveUp_;
    EventProperty moveDown_;
    EventProperty moveForward_;
    EventProperty moveBackward_;
    EventProperty moveLeft_;
    EventProperty moveRight_;

    EventProperty stepRotateUp_;
    EventProperty stepRotateDown_;
    EventProperty stepRotateLeft_;
    EventProperty stepRotateRight_;

    EventProperty stepZoomIn_;
    EventProperty stepZoomOut_;

    EventProperty stepPanUp_;
    EventProperty stepPanDown_;
    EventProperty stepPanLeft_;
    EventProperty stepPanRight_;

    EventProperty touchGesture_;

private:
    auto props() {
        return std::tie(trackballMethod_, sensitivity_, movementSpeed_, fixUp_, worldUp_,
                        customWorldUp_, verticalAngleLimit_, handleInteractionEvents_,
                        allowHorizontalPanning_, allowVerticalPanning_, boundedPanning_,
                        allowZooming_, allowWheelZooming_, boundedZooming_,
                        allowHorizontalRotation_, allowVerticalRotation_,
                        allowViewDirectionRotation_, allowRecenterView_, animate_);
    }
    auto props() const {
        return std::tie(trackballMethod_, sensitivity_, movementSpeed_, fixUp_, worldUp_,
                        customWorldUp_, verticalAngleLimit_, handleInteractionEvents_,
                        allowHorizontalPanning_, allowVerticalPanning_, boundedPanning_,
                        allowZooming_, allowWheelZooming_, boundedZooming_,
                        allowHorizontalRotation_, allowVerticalRotation_,
                        allowViewDirectionRotation_, allowRecenterView_, animate_);
    }

    auto eventprops() {
        return std::tie(mouseRecenterFocusPoint_, wheelZoom_, mouseRotate_, mouseZoom_, mousePan_,
                        mouseReset_, moveUp_, moveDown_, moveForward_, moveBackward_, moveLeft_,
                        moveRight_, stepRotateUp_, stepRotateDown_, stepRotateLeft_,
                        stepRotateRight_, stepZoomIn_, stepZoomOut_, stepPanUp_, stepPanDown_,
                        stepPanLeft_, stepPanRight_, touchGesture_);
    }
    auto eventprops() const {
        return std::tie(mouseRecenterFocusPoint_, wheelZoom_, mouseRotate_, mouseZoom_, mousePan_,
                        mouseReset_, moveUp_, moveDown_, moveForward_, moveBackward_, moveLeft_,
                        moveRight_, stepRotateUp_, stepRotateDown_, stepRotateLeft_,
                        stepRotateRight_, stepZoomIn_, stepZoomOut_, stepPanUp_, stepPanDown_,
                        stepPanLeft_, stepPanRight_, touchGesture_);
    }
};

}  // namespace inviwo
