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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/intersection/raysphereintersection.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/interaction/trackballobject.h>

#include <math.h>

namespace inviwo {

class IVW_CORE_API Trackball : public CompositeProperty {
public:
    /**
     * Rotates and moves object around a sphere.
     * The trackball does not take ownership of pointers handed to it.
     * @see CameraTrackball
     */
    Trackball(TrackballObject* object);
    Trackball(const Trackball& rhs);
    Trackball& operator=(const Trackball& that);
    virtual ~Trackball();

    virtual void invokeEvent(Event* event) override;

    const vec3& getLookTo() const;
    const vec3& getLookFrom() const;
    const vec3& getLookUp() const;

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
     * @param vec3 lookFrom
     * @param vec3 lookTo
     * @param vec3 lookUp
     */
    void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp);

    vec3 getWorldSpaceTranslationFromNDCSpace(const vec3& fromNormalizedDeviceCoord,
                                              const vec3& toNormalizedDeviceCoord);

protected:
    enum class Direction { Up = 0, Left, Down, Right };

    vec3 mapNormalizedMousePosToTrackball(const vec2& mousePos, float radius = 1.0f);
    void rotateTrackBall(const vec3& fromTrackballPos, const vec3& toTrackballPos);
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
    void animate();

    TrackballObject* object_;
    bool isMouseBeingPressedAndHold_;

    vec2 lastMousePos_;
    double gestureStartNDCDepth_;
    float trackBallWorldSpaceRadius_;

    // Interaction restrictions
    BoolProperty handleInteractionEvents_;
    // Options to restrict translation along view-space axes.
    BoolProperty allowHorizontalPanning_;  ///< Enable/disable horizontal panning
    BoolProperty allowVerticalPanning_;    ///< Enable/disable vertical panning
    BoolProperty allowZooming_;            ///< Enable/disable zooming

    DoubleProperty maxZoomInDistance_;     ///< Cannot zoom in closer than this distance
    // Options to restrict rotation around view-space axes.
    BoolProperty allowHorizontalRotation_;  ///< Enable/disable rotation around horizontal axis
    BoolProperty allowVerticalRotation_;    ///< Enable/disable rotation around vertical axis
    
    // Enable/disable rotation around view direction axis
    BoolProperty  allowViewDirectionRotation_;  

    BoolProperty animate_;

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

    const float radius = 0.5f;  ///< Radius in normalized screen space [0 1]^2
    const float stepsize = 0.05f;

    glm::quat lastRot_;
    std::chrono::system_clock::time_point lastRotTime_;
    bool evaluated_;
    Timer timer_;
};

}

#endif  // IVW_TRACKBALL_H
