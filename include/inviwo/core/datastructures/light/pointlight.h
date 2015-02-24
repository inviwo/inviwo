/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_POINT_LIGHT_H
#define IVW_POINT_LIGHT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/baselightsource.h>

namespace inviwo {

class PointLight: public LightSource {
public:
    PointLight(): LightSource() { setFieldOfView(static_cast<float>(2.*M_PI)); }
    virtual ~PointLight() {};
    virtual PointLight* clone() const { return new PointLight(*this); }

    virtual float getArea() const { return 4.f*static_cast<float>(M_PI)*size_.x*size_.y; }
    /**
     * Get radiant flux (color) of light source.
     * @see setPower
     * @return Radiant flux in watt.
     */
    virtual vec3 getPower() const { return getIntensity()*getArea(); }


    LightSourceType::Enum getLightSourceType() const { return LightSourceType::LIGHT_POINT; }

    /**
     * Get world position of light source.
     *
     * @return World position of light source.
     */
    vec3 getPosition() const { return getCoordinateTransformer().getModelToWorldMatrix()[3].xyz(); }

    /**
     * Set world position of light source.
     *
     * @param position World position of light source.
     */
    void setPosition(const vec3& position) { getWorldMatrix()[3].xyz() = position; }

protected:
    vec3 position_;

};

} // namespace inviwo

#endif // IVW_POINT_LIGHT_H