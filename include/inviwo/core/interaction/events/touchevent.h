/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#ifndef IVW_TOUCHEVENT_H
#define IVW_TOUCHEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/touchstate.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class IVW_CORE_API TouchPoint {
public:
    TouchPoint() = default;
    /**
     * @param id Touch point id that distinguishes a particular touch point
     * @param pos Position in screen coordinates [0 dim-1]^2.
     * @param posNormalized Position normalized to the size of the screen [0 1]^2.
     * @param prevPos Previous position in screen coordinates [0 dim-1]^2.
     * @param  prevPosNormalized Previous position normalized to the size of the screen [0 1]^2.
     * @param touchState State of the touch point.
     * @param depth Depth value in normalized device coordinates ([-1 1]) at touch point, 1
     * if no depth is available.
     */
    TouchPoint(int id, TouchState touchState, dvec2 posNormalized, dvec2 prevPosNormalized,
               dvec2 pressedPosNormalized, uvec2 canvasSize, double pressure, double depth = 1.0);

    TouchState state() const;
    /**
    * \brief Retrieve touch point id
    * @return int
    */
    int id() const;
    void setId(int id);
    /**
     * \brief Retrieve position in screen coordinates [0 dim-1]^2
     */
    dvec2 pos() const;
    void setPos(dvec2 val);
    /**
    * \brief Retrieve position normalized to the size of the screen [0 1]^2.
    */
    dvec2 posNormalized() const;
    void setPosNormalized(dvec2 val);
    /**
    * \brief Retrieve the previous event position in screen coordinates [0 dim-1]^2
    */
    dvec2 prevPos() const;
    void setPrevPos(dvec2 val);
    /**
    * \brief Retrieve the previous position normalized to the size of the screen [0 1]^2.
    */
    dvec2 prevPosNormalized() const;
    void setPrevPosNormalized(dvec2 val);

    /**
    * \brief Retrieve the pressed event position in screen coordinates [0 dim-1]^2
    */
    dvec2 pressedPos() const;
    void setPressedPos(dvec2 val);

    /**
    * \brief Retrieve the pressed position normalized to the size of the screen [0 1]^2.
    */
    dvec2 pressedPosNormalized() const;
    void setPressedPosNormalized(dvec2 val);

    /**
    * Retrieve depth value in normalized device coordinates at touch point.
    * Defined in [-1 1], where -1 is the near plane and 1 is the far plane.
    * Will be 1 if no depth value is available.
    */
    double depth() const;
    void setDepth(double val);

    double pressure() const;
    void setPressure(double val);

    uvec2 canvasSize() const;
    void setCanvasSize(uvec2 size);

    /**
     * Returns the normalized device coordinates. Position and depth normalized to the range of
     * (-1,1) In in a left handed coordinate system.  The lower left near will be (-1,-1,-1)
     * And the upper right far (1,1,1)
     */
    dvec3 ndc() const;

protected:
    int id_;
    TouchState state_;
    dvec2 posNormalized_;
    dvec2 prevPosNormalized_;
    dvec2 pressedPosNormalized_;
    double pressure_;
    uvec2 canvasSize_;
    double depth_;
};

class IVW_CORE_API TouchEvent : public InteractionEvent {
public:
    TouchEvent();
    TouchEvent(const std::vector<TouchPoint>& touchPoints);

    virtual TouchEvent* clone() const override;
    virtual ~TouchEvent() = default;

    bool hasTouchPoints() const;

    const std::vector<TouchPoint>& touchPoints() const;
    std::vector<TouchPoint>& touchPoints();

    void setTouchPoints(std::vector<TouchPoint> val);

    uvec2 canvasSize() const;

    /**
     * \brief Computes average position. Returns dvec2(0) if no touch points exist.
     * @return dvec2 sum(touch points) / nPoints
     */
    dvec2 centerPoint() const;

    /**
    * \brief Computes average normalized position. Returns dvec2(0) if no touch points exist.
    * @return dvec2 sum(touch points) / nPoints
    */
    dvec2 centerPointNormalized() const;

    /**
    * \brief Computes previous average normalized position. Returns dvec2(0) if no touch points
    * exist.
    * @return dvec2 sum(touch points) / nPoints
    */
    dvec2 prevCenterPointNormalized() const;

    dvec3 centerNDC() const;

    double averageDepth() const;

    static PickingState getPickingState(const std::vector<TouchPoint>& points);

    /**
    * \brief Retrieve pointers to the two closest touch points
    * @return std::vector<const TouchPoint*>, pointers to the two closest touch points
    *  vector can have less then two elements, which indicate that not enough points exist
    */
    std::vector<const TouchPoint*> findClosestTwoTouchPoints() const;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() { return util::constexpr_hash("org.inviwo.TouchEvent"); }

private:
    std::vector<TouchPoint> touchPoints_;
};

}  // namespace

#endif  // IVW_TOUCHEVENT_H