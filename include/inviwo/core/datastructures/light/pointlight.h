/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
#include <glm/gtc/constants.hpp>

namespace inviwo {

class IVW_CORE_API PointLight : public LightSource {
public:
    PointLight() : LightSource() { setFieldOfView(glm::two_pi<float>()); }
    virtual ~PointLight() = default;
    virtual PointLight* clone() const override;

    virtual float getArea() const override { return 4.f * glm::pi<float>() * size_.x * size_.y; }
    /**
     * Get radiant flux (color) of light source.
     * @see setPower
     * @return Radiant flux in watt.
     */
    virtual vec3 getPower() const override { return getIntensity() * getArea(); }

    LightSourceType getLightSourceType() const override { return LightSourceType::Point; }

    /**
     * Get world position of light source.
     *
     * @return World position of light source.
     */
    vec3 getPosition() const { return vec3(getCoordinateTransformer().getModelToWorldMatrix()[3]); }

    /**
     * Set world position of light source.
     *
     * @param position World position of light source.
     */
    void setPosition(const vec3& position) {
        worldMatrix_[3][0] = position[0];
        worldMatrix_[3][1] = position[1];
        worldMatrix_[3][2] = position[2];
    }

    virtual Document getInfo() const override;

protected:
    vec3 position_{0.0f};
};

}  // namespace inviwo
