/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_BASELIGHT_H
#define IVW_BASELIGHT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/spatialdata.h>

namespace inviwo {

namespace LightSourceType {
enum Enum { LIGHT_AREA = 0, LIGHT_CONE, LIGHT_POINT, LIGHT_DIRECTIONAL };
}
// TODO: Change/add transformation and size information to meters instead of texture space.

class IVW_CORE_API LightSource : public SpatialEntity<3> {
public:
    LightSource() : fieldOfView_(static_cast<float>(0.5 * M_PI)){};
    virtual ~LightSource(){};

    virtual float getArea() const = 0;

    /**
     * Get radiant flux (color) of light source.
     * @return Radiant flux in watt.
     */
    virtual vec3 getPower() const = 0;

    /**
     * Get the intensity (color) from the light source given in watt per steradian (flux density per
     *solid angle, W*s*r^-1).
     *
     * @return Flux density per solid angle, W*s*r^-1
     */
    const vec3 getIntensity() const { return intensity_; }

    /**
     * Set the intensity (color) from the light source given in watt per steradian (flux density per
     *solid angle, W*s*r^-1).
     *
     * @param intensity
     */
    void setIntensity(const vec3& intensity) { intensity_ = intensity; }

    virtual LightSourceType::Enum getLightSourceType() const = 0;

    /**
     * Return field of view in radians.
     *
     * @return Field of view in radians
     */
    float getFieldOfView() const { return fieldOfView_; }

    void setFieldOfView(float FOVInRadians) { fieldOfView_ = FOVInRadians; }

    /**
     * Get width and height in world space.
     *
     * @return
     */
    const vec2& getSize() const { return size_; }

    /**
     * Set width and height in texture space.
     *
     * @param newSize
     */
    void setSize(const vec2& newSize) { size_ = newSize; }

    /**
     * Get is enabled.
     *
     * @return
     */
    bool isEnabled() const { return enabled_; }

    /**
     * Set if enabled.
     *
     * @param enable
     */
    void setEnabled(bool enable) { enabled_ = enable; }

    virtual std::string getDataInfo() const { return "LightSource"; }

    static uvec3 COLOR_CODE;

protected:
    vec3 intensity_;     // Color of light source, flux density per solid angle (given in watt per
                         // steradian W*s*r^-1)
    float fieldOfView_;  // Field of view in radians
    vec2 size_;          // width, height in world space
    bool enabled_;
};

/** 
 * \brief Encodes the position and direction in a matrix.
 * 
 * Light source position is extracted using:
 * p = M * vec4(0, 0, 0, 1) 
 * And the light source direction using:
 * d = normalize(M * vec4(0, 0, 1, 0)) 
 * @param vec3 pos Light source position.
 * @param vec3 dir Light source direction.
 * @return IVW_CORE_API mat4 Transformation from light source model space to world space.
 */
IVW_CORE_API mat4 getLightTransformationMatrix(vec3 pos, vec3 dir);

}  // namespace inviwo

#endif  // IVW_BASELIGHT_H
