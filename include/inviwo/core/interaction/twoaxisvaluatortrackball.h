/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_TWOAXISVALUATORTRACKBALL_H
#define IVW_TWOAXISVALUATORTRACKBALL_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/properties/cameraproperty.h>

namespace inviwo {

/**
 * \class TwoAxisValuatorTrackball
 * \brief Implements a Two Axis Valuator Trackball
 *
 * The Two Axis Valuator Trackball interprets horizontal mouse moves as rotation around the camera up vector
 * and vertical moves as rotation around the camera right vector. This also binds zoom to the mouse wheel.
 * fixUp_ maps horizontal moves to rotations around the world up vector instead when True.
 * v_angle_limit_ restricts the angle between world up and the view direction, restricting vertical rotation
 * so that the camera up vector never points in -world up direction.
 * Note that the basic Trackball functionality (pan, restrict axes, etc.) still works.
 */
class IVW_CORE_API TwoAxisValuatorTrackball : public Trackball {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    TwoAxisValuatorTrackball(CameraProperty* camProp);
    ~TwoAxisValuatorTrackball() = default;
    void rotate(Event* event);
    void zoomWheel(Event * event);

    FloatProperty sensitivity_; /// Controls the rotation sensitivity
    FloatProperty v_angle_limit_; /// Limits the angle between world up and view direction
    BoolProperty fixUp_; /// Horizontal mouse moves rotate around world up when true and around camera up when false
};

}  // namespace inviwo

#endif  // IVW_TWOAXISVALUATORTRACKBALL_H
