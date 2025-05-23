/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/util/fmtutils.h>

#include <iosfwd>

namespace inviwo {

class Document;

enum class LightSourceType { Area = 0, SpotLight, Point, Directional };

IVW_CORE_API std::string_view enumToStr(LightSourceType lt);

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, LightSourceType lt);

class IVW_CORE_API LightSource : public SpatialEntity {
public:
    LightSource();
    virtual ~LightSource() = default;

    virtual float getArea() const = 0;

    /**
     * Get radiant flux (color) of light source.
     * @return Radiant flux in watt.
     */
    virtual vec3 getPower() const = 0;

    /**
     * Get the intensity (color) from the light source given in watt per steradian (flux density per
     * solid angle, W*s*r^-1).
     * @return Flux density per solid angle, W*s*r^-1
     */
    vec3 getIntensity() const;

    /**
     * Set the intensity (color) from the light source given in watt per steradian (flux density per
     * solid angle, W*s*r^-1).
     * @param intensity
     */
    void setIntensity(const vec3& intensity);

    virtual LightSourceType getLightSourceType() const = 0;

    /**
     * Return field of view in radians.
     * @return Field of view in radians
     */
    float getFieldOfView() const;

    void setFieldOfView(float FOVInRadians);

    /**
     * Get width and height in world space.
     */
    const vec2& getSize() const;

    /**
     * Set width and height in texture space.
     */
    void setSize(const vec2& newSize);

    /**
     * Get is enabled.
     */
    bool isEnabled() const;

    /**
     * Set if enabled.
     */
    void setEnabled(bool enable);

    virtual const Axis* getAxis(size_t) const override { return nullptr; }

    virtual Document getInfo() const;

    static constexpr uvec3 colorCode{128, 64, 196};
    static constexpr std::string_view classIdentifier{"org.inviwo.LightSource"};
    static constexpr std::string_view dataName{"LightSource"};

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
 * `p = M * vec4(0, 0, 0, 1)`
 * And the light source direction using:
 * `d = normalize(M * vec4(0, 0, 1, 0))`
 * @param pos Light source position.
 * @param dir Light source direction.
 * @return Transformation from light source model space to world space.
 */
IVW_CORE_API mat4 getLightTransformationMatrix(vec3 pos, vec3 dir);

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::LightSourceType> : inviwo::FlagFormatter<inviwo::LightSourceType> {};
#endif
