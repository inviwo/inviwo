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

#ifndef IVW_STRUCTS_GLSL
#define IVW_STRUCTS_GLSL

struct CameraParameters {
   mat4 worldToView; // Equivalent to view
   mat4 viewToWorld; // Equivalent to viewInverse
   mat4 viewToClip; // Equivalent to projection
   mat4 clipToView; // Equivalent to projectionInverse
   mat4 worldToClip; // Equivalent to viewProjection
   mat4 clipToWorld; // Equivalent to viewProjectionInverse
   vec3 position;
   float nearPlane;  // zNear
   float farPlane;   // zFar
};

// Convenience functions to retrieve camera parameters
vec3 right(in CameraParameters camera) { return normalize(camera.viewToWorld[0].xyz); }
vec3 up(in CameraParameters camera) { return normalize(camera.viewToWorld[1].xyz); }
vec3 direction(in CameraParameters camera) { return normalize(camera.viewToWorld[2].xyz); }

struct GeometryParameters {
    mat4 dataToModel;
    mat4 modelToData;
    mat4 dataToWorld;
    mat4 worldToData;
    mat4 modelToWorld;
    mat4 worldToModel;
    mat3 modelToWorldNormalMatrix; // Equivalent to normalMatrix
    mat3 dataToWorldNormalMatrix;  // Equivalent to normalMatrix
};

struct VolumeParameters {
    mat4 dataToModel;
    mat4 modelToData;
    mat4 dataToWorld;
    mat4 worldToData;
    mat4 modelToWorld;
    mat4 worldToModel;
    mat4 worldToTexture;
    mat4 textureToWorld;
    mat4 textureToWorldNormalMatrix;       // Equivalent to the inverse transposed textureToWorld matrix
    mat4 textureToIndex;                   // Transform from [0 1] to [-0.5 dim-0.5]
    mat4 indexToTexture;                   // Transform from [-0.5 dim-0.5] to [0 1]
    mat3 textureSpaceGradientSpacing;      // Maximum possible distance to go without ending up
                                           // outside of a voxel (half of minimum voxel spacing
                                           // for volumes with orthogonal basis)
    vec3 dimensions;                       // Number of voxels (dim) per axis
    vec3 reciprocalDimensions;             // 1 over the number of voxels
    vec3 worldSpaceGradientSpacing;       // Spacing between gradient samples in world space
    float formatScaling;                   // This scaling and offset parameters is used to
    float formatOffset;                    // map value from data range [min,max] to [0,1]
    float signedFormatScaling;             // or to [-1,1] for signed data. It is used by
    float signedFormatOffset;              // sampler3d.glsl, and is calculated in shaderutils.cpp
};

struct ImageParameters {
    mat4 dataToModel;
    mat4 modelToData;
    mat4 dataToWorld;
    mat4 worldToData;
    mat4 modelToWorld;
    mat4 worldToModel;
    mat4 worldToTexture;
    mat4 textureToWorld;
    mat4 textureToIndex;  // Transform from [0 1] to [-0.5 dim-0.5]
    mat4 indexToTexture;  // Transform from [-0.5 dim-0.5] to [0 1]
    vec2 dimensions;
    vec2 reciprocalDimensions;

    float formatScaling;                   // This scaling and offset parameters is used to
    float formatOffset;                    // map value from data range [min,max] to [0,1]
    float signedFormatScaling;             // or to [-1,1] for signed data. It is used by
    float signedFormatOffset;              // sampler2d.glsl, and is calculated in shaderutils.cpp
};

struct LightParameters {
    vec3 position;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float specularExponent;
};

struct MaterialColors {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct ShadingParameters {
    MaterialColors colors;
    vec3 normal;
    vec3 worldPosition;
    vec3 lightIntensity; // incident light intensity sampled from a light volume at current position
};

struct RaycastingParameters {
    float samplingRate;
    float isoValue;
};

struct PlaneParameters {
    vec3 position;
    vec3 normal;
    vec4 color;
};

struct VolumeIndicatorParameters {
    PlaneParameters plane1;
    PlaneParameters plane2;
    PlaneParameters plane3;
};

struct StipplingParameters {
    float len;
    float spacing;
    float offset;
    float worldScale;
};

// Conversion factors for conversions from [0,1] obtained from getNormalizedTexel/Voxel() to 
// value space and to the OpenGL output range
// see glformatsutil.h
struct GLFormatConversion {
    // used for conversion from normalized values [0,1] to value space
    float toValueScaling;
    float toValueOffset;

    // conversion from value space to OpenGL output range basd on output texture format 
    // (regular, normalized, sign normalized)
    float outputValueOffset;
    float outputScaling;
    float outputOffset;
}; 

#endif // IVW_STRUCTS_GLSL