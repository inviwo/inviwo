/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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
#include <inviwo/core/datastructures/light/baselightsource.h>

namespace inviwo {

class IVW_CORE_API DirectionalLight : public LightSource {
public:
    DirectionalLight() = default;
    virtual ~DirectionalLight() = default;
    virtual DirectionalLight* clone() const override;

    virtual float getArea() const override { return size_.x * size_.y; }
    /**
     * Get radiant flux (color) of light source.
     * @see setPower
     * @return Radiant flux in watt.
     */
    virtual vec3 getPower() const override { return getIntensity() * getArea() * glm::pi<float>(); }

    LightSourceType getLightSourceType() const override { return LightSourceType::directional; }

    /**
     * Get normalized general direction of light source.
     * @return Normalized direction of light source.
     */
    const vec3& getDirection() const { return direction_; }

    /**
     * Set normalized direction of light source.
     * @param direction Normalized direction of light source.
     */
    void setDirection(const vec3& direction) { direction_ = direction; }

protected:
    vec3 direction_{1.0f, 0.0f, 0.0f};
};

}  // namespace inviwo
