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
#include <inviwo/core/interaction/action.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class Trackball;
/** \class TrackballObserver
 * Trackball observer that gets notified when positions and directions change.
 * @see TrackballObservable
 */
class IVW_CORE_API TrackballObserver : public Observer {
public:
    TrackballObserver() : Observer(){};
    virtual void onAllTrackballChanged(const Trackball* trackball){};
    virtual void onLookFromChanged(const Trackball* trackball){};
    virtual void onLookToChanged(const Trackball* trackball){};
    virtual void onLookUpChanged(const Trackball* trackball){};
};

/** \class TrackballObservable
 * Can call notifyObserversInvalidationBegin and notifyObserversInvalidationEnd
 * @see TrackballObserver
 */
class IVW_CORE_API TrackballObservable : public Observable<TrackballObserver> {
public:
    TrackballObservable();
    void notifyAllChanged(const Trackball* trackball) const;
    void notifyLookFromChanged(const Trackball* trackball) const;
    void notifyLookToChanged(const Trackball* trackball) const;
    void notifyLookUpChanged(const Trackball* trackball) const;
};

class IVW_CORE_API Trackball : public CompositeProperty,
                               public TrackballObservable {
public:
    InviwoPropertyInfo();
    /**
     * Rotates and moves object around a sphere.
     * This object does not take ownership of pointers handed to it.
     * Use TrackballObserver to receive notifications when the data has changed.
     * @see TrackballCamera
     * @param vec3 * lookFrom Look from position
     * @param vec3 * lookTo Look to position
     * @param vec3 * lookUp Normalized look up direction vector
     */
    Trackball(vec3* lookFrom, vec3* lookTo, vec3* lookUp);
    virtual ~Trackball();

    virtual void invokeEvent(Event* event) override;

    vec3* getLookTo() { return lookTo_; }
    vec3* getLookFrom() { return lookFrom_; }
    vec3* getLookUp() { return lookUp_; }
    const vec3* getLookTo() const { return lookTo_; }
    const vec3* getLookFrom() const { return lookFrom_; }
    const vec3* getLookUp() const { return lookUp_; }

protected:
    void setPanSpeedFactor(float psf);

private:
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
    
    void pinchGesture(Event* event);
    void panGesture(Event* event);

    float pixelWidth_;
    float panSpeedFactor_;
    bool isMouseBeingPressedAndHold_;

    vec2 lastMousePos_;
    vec3 lastTrackballPos_;

    vec3* lookFrom_;
    vec3* lookTo_;
    vec3* lookUp_;

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
    
    EventProperty pinchGesture_;
    EventProperty panGesture_;
};
}

#endif  // IVW_TRACKBALL_H