/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
enum Enum {
    LIGHT_AREA = 0,
    LIGHT_CONE,
    LIGHT_POINT,
    LIGHT_DIRECTIONAL
};
}
// TODO: Change/add transformation and size information to meters instead of texture space.

class IVW_CORE_API LightSource: public SpatialEntity<3> {
public:
    LightSource(): fieldOfView_(static_cast<float>(0.5*M_PI)) {};
    virtual ~LightSource() {};

    virtual float getArea() const = 0;

    /**
     * Get radiant flux (color) of light source.
     * @return Radiant flux in watt.
     */
    virtual vec3 getPower() const = 0;

    /**
     * Get the intensity (color) from the light source given in watt per steradian (flux density per solid angle, W*s*r^-1).
     *
     * @return Flux density per solid angle, W*s*r^-1
     */
    const vec3 getIntensity() const { return intensity_; }

    /**
     * Set the intensity (color) from the light source given in watt per steradian (flux density per solid angle, W*s*r^-1).
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

protected:
    vec3 intensity_; // Color of light source, flux density per solid angle (given in watt per steradian W*s*r^-1)
    float fieldOfView_; // Field of view in radians
    vec2 size_; // width, height in world space
    bool enabled_;
};

// Data type that can be transfered to OpenCL device
// Must be same as modules/opencl/cl/light.cl
// Note that largest variables should be placed first
// in order to ensure struct size
typedef struct {
    mat4 tm; // Transformation matrix from local to world coordinates
    vec4 radiance; // cl_float3 == cl_float4
    vec2 size; // width, height
    int type; // LightSourceType, use integer to handle size of struct easier
    float area; // area of light source
    float cosFOV; // cos( (field of view)/2 ), used by cone light

    int padding[7]; // OpenCL requires sizes that are power of two (32, 64, 128 and so on)
} PackedLightSource;

// Transform a BaseLightSource to PackedLightSource
IVW_CORE_API PackedLightSource baseLightToPackedLight(const LightSource* lightsource, float radianceScale);

// Transform a BaseLightSource to PackedLightSource and apply the transformation matrix to the light source transformation matrix
IVW_CORE_API PackedLightSource baseLightToPackedLight(const LightSource* lightsource, float radianceScale, const mat4& transformLightMat);

// Calculate how many samples to take from each light source.
// x component contains the amount of samples to take in x and y dimensions
// y component is the number of samples taken for each light source (x*x)
IVW_CORE_API uvec2 getSamplesPerLight(uvec2 nSamples, int nLightSources);

// Calculate the object to texture transformation matrix for the light
IVW_CORE_API mat4 getLightTransformationMatrix(vec3 pos, vec3 dir);


} // namespace inviwo

#endif // IVW_BASELIGHT_H
