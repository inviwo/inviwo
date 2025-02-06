out vec4 FragData1;

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
    float length;
    float spacing;
    float offset;
    float worldScale;
};

#endif // IVW_STRUCTS_GLSL
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

#ifndef IVW_SAMPLER3D_GLSL
#define IVW_SAMPLER3D_GLSL



//
// Fetch texture data using texture coordinates [0,1]
//

// Return a the raw texture
vec4 getVoxel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    return (texture(volume, samplePos));
}
// Return a value mapped from data range [min,max] to [0,1]
// The data range [min, max] here is the range specified by the dataMap_.dataRange of volume
// and does not always match the range of the specified data type.
// We have to apply some scaling here to compensate for the fact that the data range of the data is
// not the same as the min/max of the data type. And at the same time take into account that OpenGL
// also does its own normalization, which if different for floating point and integer types
// see: https://www.opengl.org/wiki/Normalized_Integer
// the actual calculation of the scaling parameters is done in volumeutils.cpp
vec4 getNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    return (texture(volume, samplePos) + volumeParams.formatOffset)
        * (1.0 - volumeParams.formatScaling);
}

float getNormalizedVoxelChannel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos,int channel) {
    vec4 v = getNormalizedVoxel(volume,volumeParams,samplePos);
    return v[channel];
}


// Return a value mapped from data range [min,max] to [-1,1]
// Same as getNormalizedVoxel but for signed types. 
vec4 getSignNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    return (texture(volume, samplePos) + volumeParams.signedFormatOffset)
        * (1.0 - volumeParams.signedFormatScaling);
}


//
// Fetch texture data using texture indices [0,N]
//

// Return a the raw texture
vec4 getVoxel(sampler3D volume, VolumeParameters volumeParams, ivec3 samplePos) {
#ifdef GLSL_VERSION_140
    return texelFetch(volume, samplePos, 0);
#else
    return texture(volume, samplePos);
#endif
}
// Return a value mapped from data range [min,max] to [0,1]
vec4 getNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, ivec3 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(volume, samplePos, 0) + volumeParams.formatOffset)
        * (1.0 - volumeParams.formatScaling);
#else
    return (texture(volume, samplePos) + volumeParams.formatOffset)
        * (1.0 - volumeParams.formatScaling);
#endif
}
// Return a value mapped from data range [min,max] to [-1,1]
vec4 getSignNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, ivec3 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(volume, samplePos, 0) + volumeParams.signedFormatOffset)
        * (1.0 - volumeParams.signedFormatScaling);
#else
    return (texture(volume, samplePos) + volumeParams.signedFormatOffset)
        * (1.0 - volumeParams.signedFormatScaling);
#endif
}

#endif  // IVW_SAMPLER3D_GLSL

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

#ifndef IVW_DEPTH_GLSL
#define IVW_DEPTH_GLSL



float convertScreenToEye(CameraParameters camera, float depthScreen) {
    float depthNDC = 2.0 * depthScreen - 1.0;
    float depthEye = 2.0 * camera.nearPlane * camera.farPlane /
                     (camera.farPlane + camera.nearPlane - depthNDC * (camera.farPlane - camera.nearPlane));
    return depthEye;
}

float convertEyeToScreen(CameraParameters camera, float depthEye) {
    float A = -(camera.farPlane+camera.nearPlane)/(camera.farPlane-camera.nearPlane);
    float B = (-2.0*camera.farPlane*camera.nearPlane) / (camera.farPlane-camera.nearPlane);
    float depthScreen = 0.5*(-A*depthEye + B) / depthEye + 0.5;
    return depthScreen;
}

float calculateDepthValue(CameraParameters camera, float t, float entryDepthScreen, float exitDepthScreen) {
    // to calculate the correct depth values, which are not linear in the depth buffer,
    // we must first convert our screen space coordinates into eye coordinates and interpolate there.
    // transform into eye space
    float entryDepthEye = convertScreenToEye(camera, entryDepthScreen);
    float exitDepthEye  = convertScreenToEye(camera, exitDepthScreen);
    // compute the depth value in clip space
    float resultEye = entryDepthEye + t * (exitDepthEye - entryDepthEye);
    // transform back to screen space
    float resultScreen = convertEyeToScreen(camera, resultEye);
    return resultScreen;
}



float calculateTValueFromDepthValue(CameraParameters camera, float depth, float entryDepthScreen, float exitDepthScreen) {
    // to calculate the correct depth values, which are not linear in the deph buffer,
    // we must first convert our screen space coordinates into eye coordinates and interpolate there.
    // transform into eye space
    float entryDepthEye = convertScreenToEye(camera, entryDepthScreen);
    float exitDepthEye  = convertScreenToEye(camera, exitDepthScreen);
    float depthEye  = convertScreenToEye(camera, depth);
    // compute the depth value in clip space
    return (depthEye - entryDepthEye) / (exitDepthEye - entryDepthEye);
}


float convertDepthViewToClip(CameraParameters camera, float z) {
    // convert linear depth from view coordinates to non-linear clip coords [-1,1]
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return (Zf + Zn) / (Zf - Zn) + (-2.0 * Zf * Zn) / (z * (Zf - Zn));
}

float convertDepthClipToView(CameraParameters camera, float z) {
    // convert non-linear depth from clip coords [-1,1] back to linear view coords (-inf,inf)
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return 2.0 * Zn * Zf / (Zf + Zn - z * (Zf - Zn));
}

float convertDepthScreenToView(CameraParameters camera, float z) {
    // convert non-linear depth from screen coords [0,1] back to linear view coords (-inf,inf)
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return Zn*Zf / (Zf - z*(Zf - Zn));
}

float convertDepthViewToScreen(CameraParameters camera, float z) {
    // convert linear depth from view coordinates to non-linear screen coords [0,1]
    return (convertDepthViewToClip(camera, z) + 1.0) * 0.5;
}

#endif // IVW_DEPTH_GLSL


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

#ifndef IVW_GRADIENTS_GLSL
#define IVW_GRADIENTS_GLSL



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

#ifndef IVW_SAMPLER2D_GLSL
#define IVW_SAMPLER2D_GLSL



vec4 textureLookup2Dnormalized(sampler2D tex, ImageParameters textureParams, vec2 samplePos) {
    return texture(tex, samplePos);
}

vec4 textureLookup2Dscreen(sampler2D tex, ImageParameters textureParams, vec2 samplePos) {
    return texture(tex, samplePos*textureParams.reciprocalDimensions);
}

// Return a the raw texture
vec4 getTexel(sampler2D image, ImageParameters imageParams, vec2 samplePos) {
    return texture(image, samplePos);
}
// Return a value mapped from data range [min,max] to [0,1]
// The data range [min, max] here is the range specified by the dataMap_.dataRange of image
// and does not always match the range of the specified data type.
// We have to apply some scaling here to compensate for the fact that the data range of the data is
// not the same as the min/max of the data type. And at the same time take into account that OpenGL
// also does its own normalization, which if different for floating point and integer types
// see: https://www.opengl.org/wiki/Normalized_Integer
// the actual calculation of the scaling parameters is done in imageutils.cpp
vec4 getNormalizedTexel(sampler2D image, ImageParameters imageParams, vec2 samplePos) {
    return (texture(image, samplePos) + imageParams.formatOffset)
        * (1.0 - imageParams.formatScaling);
}

float getNormalizedTexelChannel(sampler2D image, ImageParameters imageParams, vec2 samplePos,int channel) {
    vec4 v = getNormalizedTexel(image,imageParams,samplePos);
    return v[channel];
}


// Return a value mapped from data range [min,max] to [-1,1]
// Same as getNormalizedTexel but for signed types. 
vec4 getSignNormalizedTexel(sampler2D image, ImageParameters imageParams, vec2 samplePos) {
    return (texture(image, samplePos) + imageParams.signedFormatOffset)
        * (1.0 - imageParams.signedFormatScaling);
}


//
// Fetch texture data using texture indices [0,N]
//

// Return a the raw texture
vec4 getTexel(sampler2D image, ImageParameters imageParams, ivec2 samplePos) {
#ifdef GLSL_VERSION_140
    return texelFetch(image, samplePos, 0);
#else
    return texture(image, samplePos);
#endif
}
// Return a value mapped from data range [min,max] to [0,1]
vec4 getNormalizedTexel(sampler2D image, ImageParameters imageParams, ivec2 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(image, samplePos, 0) + imageParams.formatOffset)
        * (1.0 - imageParams.formatScaling);
#else
    return (texture(image, samplePos) + imageParams.formatOffset)
        * (1.0 - imageParams.formatScaling);
#endif
}
// Return a value mapped from data range [min,max] to [-1,1]
vec4 getSignNormalizedTexel(sampler2D image, ImageParameters imageParams, ivec2 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(image, samplePos, 0) + imageParams.signedFormatOffset)
        * (1.0 - imageParams.signedFormatScaling);
#else
    return (texture(image, samplePos) + imageParams.signedFormatOffset)
        * (1.0 - imageParams.signedFormatScaling);
#endif
}

#endif  // IVW_SAMPLER2D_GLSL



// Compute gradient for 1 channel.

// Compute world space gradient using forward difference: f' = ( f(x+h)-f(x) ) / h
vec3 gradientForwardDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) forward differences
    // Value at f(x+h)
    vec3 fDs;
    fDs.x = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0],channel);
    fDs.y = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1],channel);
    fDs.z = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2],channel);
    // Note that this computation is performed in world space
    // f' = ( f(x+h)-f(x) ) / volumeParams.worldSpaceGradientSpacing
    return (fDs-intensity[channel])/(volumeParams.worldSpaceGradientSpacing);
}

// Compute world space gradient using central difference: f' = ( f(x+h)-f(x-h) ) / 2*h
vec3 gradientCentralDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) central differences
    vec3 cDs;
    // Value at f(x+h)
    cDs.x = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0],channel);
    cDs.y = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1],channel);
    cDs.z = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2],channel);
    // Value at f(x-h)
    cDs.x = cDs.x - getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0],channel);
    cDs.y = cDs.y - getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1],channel);
    cDs.z = cDs.z - getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2],channel);
    // Note that this computation is performed in world space
    // f' = ( f(x+h)-f(x-h) ) / 2*volumeParams.worldSpaceGradientSpacing
    return (cDs)/(2.0*volumeParams.worldSpaceGradientSpacing);
}

// Compute world space gradient using backward difference: f' = ( f(x)-f(x-h) ) / h
vec3 gradientBackwardDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) backward differences
    // Value at f(x-h)
    vec3 fDs;
    fDs.x = getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0],channel);
    fDs.y = getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1],channel);
    fDs.z = getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2],channel);
    // Note that this computation is performed in world space
    // f' = ( f(x)-f(x-h) ) / voxelSpacing
    return (intensity[channel]-fDs)/(volumeParams.worldSpaceGradientSpacing);
}

// Higher order gradients
// Compute world space gradient using higher order central difference: f' = ( -f(x+2h)+8.f(x+h)-8.f(x-h)+f(x-2h) ) / 12*h
vec3 gradientCentralDiffH(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^4) central differences
    vec3 cDs;
    // f' = ( -f(x+2h)+8.f(x+h)-8.f(x-h)+f(x-2h) ) / 12*h
    // Value at 8.f(x+h)
    cDs.x = 8.0 * getNormalizedVoxelChannel(volume, volumeParams, samplePos +volumeParams.textureSpaceGradientSpacing[0],channel);
    cDs.y = 8.0 * getNormalizedVoxelChannel(volume, volumeParams, samplePos +volumeParams.textureSpaceGradientSpacing[1],channel);
    cDs.z = 8.0 * getNormalizedVoxelChannel(volume, volumeParams, samplePos +volumeParams.textureSpaceGradientSpacing[2],channel);
    // Value at 8.f(x-h)
    cDs.x = cDs.x - 8.0 * getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0],channel);
    cDs.y = cDs.y - 8.0 * getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1],channel);
    cDs.z = cDs.z - 8.0 * getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2],channel);
    // Value at -f(x+2h)
    cDs.x = cDs.x - getNormalizedVoxelChannel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[0],channel);
    cDs.y = cDs.y - getNormalizedVoxelChannel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[1],channel);
    cDs.z = cDs.z - getNormalizedVoxelChannel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[2],channel);
    // Value at f(x+2h)
    cDs.x = cDs.x + getNormalizedVoxelChannel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[0],channel);
    cDs.y = cDs.y + getNormalizedVoxelChannel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[1],channel);
    cDs.z = cDs.z + getNormalizedVoxelChannel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[2],channel);
    // Note that this computation is performed in world space
    // f' = ( -f(x+2h)+8.f(x+h)-8.f(x-h)+f(x-2h) ) / 12*volumeParams.worldSpaceGradientSpacing
    return (cDs)/(12.0*volumeParams.worldSpaceGradientSpacing);
}

// Compute gradients for all channels.

// Compute world space gradient using forward difference: f' = ( f(x+h)-f(x) ) / h
mat4x3 gradientAllForwardDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    // Of order O(h^2) forward differences
    // Value at f(x+h)
    mat3x4 fDs;
    // Moving a fixed distance h along each xyz-axis in world space, which correspond to moving along
    // three basis vectors in texture space. 
    // This will be the minimum world space voxel spacing for volumes with orthogonal basis function.
    fDs[0] = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0]);
    fDs[1] = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1]);
    fDs[2] = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2]);
    // f' = ( f(x+h)-f(x) ) / h
    fDs -= mat3x4(intensity, intensity, intensity);
    mat4x3 gradients = transpose(fDs);
    gradients[0] /= volumeParams.worldSpaceGradientSpacing;
    gradients[1] /= volumeParams.worldSpaceGradientSpacing;
    gradients[2] /= volumeParams.worldSpaceGradientSpacing;
    gradients[3] /= volumeParams.worldSpaceGradientSpacing;
    return gradients;
}

// Compute world space gradient using central difference: f' = ( f(x+h)-f(x-h) ) / 2*h
mat4x3 gradientAllCentralDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    // Of order O(h^2) central differences
    mat3x4 cDs; 
    // Value at f(x+h)
    cDs[0] = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0]);
    cDs[1] = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1]);
    cDs[2] = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2]);
    // Value at f(x-h)
    cDs[0] -= getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0]);
    cDs[1] -= getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1]);
    cDs[2] -= getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2]);
    // f' = ( f(x+h)-f(x-h) ) / 2*h
    mat4x3 gradients = transpose(cDs);
    gradients[0] /= 2.0*volumeParams.worldSpaceGradientSpacing;
    gradients[1] /= 2.0*volumeParams.worldSpaceGradientSpacing;
    gradients[2] /= 2.0*volumeParams.worldSpaceGradientSpacing;
    gradients[3] /= 2.0*volumeParams.worldSpaceGradientSpacing;
    return gradients;
}

// Compute world space gradient using backward difference: f' = ( f(x)-f(x-h) ) / h
mat4x3 gradientAllBackwardDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    // Of order O(h^2) backward differences
    mat3x4 fDs;
    // Value at f(x-h)
    fDs[0] = getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0]);
    fDs[1] = getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1]);
    fDs[2] = getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2]);
    // f' = ( f(x)-f(x-h) ) / h
    fDs = mat3x4(intensity, intensity, intensity) - fDs;
    mat4x3 gradients = transpose(fDs);
    gradients[0] /= volumeParams.worldSpaceGradientSpacing;
    gradients[1] /= volumeParams.worldSpaceGradientSpacing;
    gradients[2] /= volumeParams.worldSpaceGradientSpacing;
    gradients[3] /= volumeParams.worldSpaceGradientSpacing;
    return gradients;
}

// Compute world space gradient using higher order central difference: f' = ( -f(x+2h)+8.f(x+h)-8.f(x-h)+f(x-2h) ) / 12*h
mat4x3 gradientAllCentralDiffH(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    // Of order O(h^4) central differences
    mat3x4 cDs;
    // Value at 8.f(x+h)
    cDs[0] = 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0]);
    cDs[1] = 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1]);
    cDs[2] = 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2]);
    // Value at 8.f(x-h)
    cDs[0] -= 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0]);
    cDs[1] -= 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1]);
    cDs[2] -= 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2]);
    // Value at -f(x+2h)
    cDs[0] -= getNormalizedVoxel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[0]);
    cDs[1] -= getNormalizedVoxel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[1]);
    cDs[2] -= getNormalizedVoxel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[2]);
    // Value at f(x+2h)
    cDs[0] += getNormalizedVoxel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[0]);
    cDs[1] += getNormalizedVoxel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[1]);
    cDs[2] += getNormalizedVoxel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[2]);
    // f' = ( -f(x+2h)+8.f(x+h)-8.f(x-h)+f(x-2h) ) / 12*h
    mat4x3 gradients = transpose(cDs);
    gradients[0] /= 12.0*volumeParams.worldSpaceGradientSpacing;
    gradients[1] /= 12.0*volumeParams.worldSpaceGradientSpacing;
    gradients[2] /= 12.0*volumeParams.worldSpaceGradientSpacing;
    gradients[3] /= 12.0*volumeParams.worldSpaceGradientSpacing;
    return gradients;
}

// Use pre-computed gradients stored in xyz of the current sample. The gradient will be transformed from texture space
// to world space by using the textureToWorldNormalMatrix, i.e. the inverse transposed textureToWorld matrix.
vec3 gradientPrecomputedXYZ(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    vec3 gradient = getVoxel(volume, volumeParams, samplePos).xyz;
    return (volumeParams.textureToWorldNormalMatrix * vec4(gradient, 0.0)).xyz;
}

// Use pre-computed gradients stored in yzw of the current sample. The gradient will be transformed from texture space
// to world space by using the textureToWorldNormalMatrix, i.e. the inverse transposed textureToWorld matrix.
vec3 gradientPrecomputedYZW(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    vec3 gradient = getVoxel(volume, volumeParams, samplePos).yzw;
    return (volumeParams.textureToWorldNormalMatrix * vec4(gradient, 0.0)).xyz;
}

// compute the partial differential for a given component using central differences
float partialDiff(sampler2D tex, ImageParameters texParams, in vec2 texcoord, 
                  in vec2 gradientTextureSpacing, in float gradientWorldSpacing, in int component) {
    float fds = getNormalizedTexel(tex, texParams, texcoord + gradientTextureSpacing)[component]
        - getNormalizedTexel(tex, texParams, texcoord - gradientTextureSpacing)[component];
    return fds / gradientWorldSpacing * 0.5;
}

#endif // IVW_GRADIENTS_GLSL


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
#ifndef IVW_SHADING_GLSL
#define IVW_SHADING_GLSL

#if !defined(APPLY_LIGHTING)
// fall-back to no shading in case APPLY_LIGHTING is not defined
#  define APPLY_LIGHTING(lighting, materialAmbientColor, materialDiffuseColor, \
    materialSpecularColor, position, normal, toCameraDir) \
    materialAmbientColor
#endif

#if !defined(SHADING_NORMAL)
// the SHADING_NORMAL macro is used to adapt the face normal for frontside, backside, or 
// two-sided shading/illumination.
// Values:
//  + 0: frontside only, no changes to input normal
//  + 1: backside only, invert normal direction
//  + 2: two-sided, adjust normal based on primitive orientation
//
// see orientedShadingNormal()
#  define SHADING_NORMAL 0
#endif



// Returns whether the primitive of the current fragment is facing toward the camera.
// Can _only_ be used in the fragment shader.
bool isFacingForward(in vec3 normal, in vec3 worldPosition) {
#if defined(__APPLE__)
    // gl_FrontFacing is not working correctly on MacOS
    // Credit: https://makc3d.wordpress.com/2015/09/17/alternative-to-gl_frontfacing/
    vec3 fdx = dFdx(worldPosition);
    vec3 fdy = dFdy(worldPosition);
    return dot(normal, cross(fdx, fdy)) > 0;
#else
    return gl_FrontFacing;
#endif
}

vec3 orientedShadingNormal(in vec3 normal, in vec3 worldPosition) {
#if defined(SHADING_NORMAL) && (SHADING_NORMAL == 0)
    return normal;
#elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
    return -normal;
#elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
    if (isFacingForward(normal, worldPosition)) {
        return normal;
    } else {
        return -normal;
    }
#else
    return normal;
#endif
}

// default material uses the supplied color for the diffuse and ambient material terms
// as well as white for the specular term (as used for volume and mesh rendering).
MaterialColors defaultMaterialColors(in vec3 diffuseColor) {
    return MaterialColors(diffuseColor, diffuseColor, vec3(1.0));
}

ShadingParameters defaultShadingParameters(in MaterialColors materialColors) {
    ShadingParameters p;
    p.colors = materialColors;
    p.normal = vec3(0);
    p.worldPosition = vec3(0);
    p.lightIntensity = vec3(0);

    return p;
}

ShadingParameters defaultShadingParameters() {
    return defaultShadingParameters(defaultMaterialColors(vec3(0)));
}
ShadingParameters shading(in vec3 diffuseColor) {
    return ShadingParameters(defaultMaterialColors(diffuseColor), vec3(0), vec3(0), vec3(0));
}
ShadingParameters shading(in vec3 diffuseColor, in vec3 normal) {
    return ShadingParameters(defaultMaterialColors(diffuseColor), normal, vec3(0), vec3(0));
}
ShadingParameters shading(in vec3 diffuseColor, in vec3 normal, in vec3 worldPosition) {
    return ShadingParameters(defaultMaterialColors(diffuseColor), normal, worldPosition, vec3(0));
}

// Calculate the diffuse term based on the Lambertian reflection model
float diffuse(in vec3 normal, in vec3 toLightDir) {
    return max(dot(normal, toLightDir), 0.0);
}

// Calculate the specular term of the Blinn-Phong model
float specularBlinnPhong(in vec3 normal, in vec3 toLightDir,
                         in vec3 toCameraDir, in float specularExponent) {
    vec3 halfway = normalize(toCameraDir + toLightDir);
    // check for special case where the light source is exactly opposite
    // to the view direction, i.e. the length of the halfway vector is zero
    if (dot(halfway, halfway) < 1.0e-6) {  // check for squared length
        return 0.0;
    } else {
        return pow(max(dot(normal, halfway), 0.0), specularExponent);
    }
}

// Calculate the specular term of the Phong model
float specularPhong(in vec3 normal, in vec3 toLightDir,
                    in vec3 toCameraDir, in float specularExponent) {
    // Compute reflection (note that glsl uses incident direction)
    // Corresponds to 2.0*dot(toLightDir, normal)*normal - toLightDir;
    vec3 r = reflect(-toLightDir, normal);
    if (dot(toLightDir, normal) < 0.0) {
        return 0.0;
    } else {
        // scale specular exponent so that it roughly matches the one of the Blinn-Phong model
        return pow(max(dot(r, toCameraDir), 0.0), specularExponent * 0.25);
    }
}

// Functions to apply different shading modes.
// All positions and directions should be in world space!
vec3 shadeAmbient(LightParameters light_, vec3 materialAmbientColor) {
    return materialAmbientColor * light_.ambientColor;
}

vec3 shadeDiffuse(LightParameters light_, vec3 materialDiffuseColor, vec3 position, vec3 normal) {
    vec3 toLightDir = normalize(light_.position - position);
    return diffuse(normal, toLightDir) * materialDiffuseColor * light_.diffuseColor;
}

vec3 shadeSpecular(LightParameters light_, vec3 materialSpecularColor, vec3 position, vec3 normal,
                   vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    return specularBlinnPhong(normal, toLightDir, toCameraDir, light_.specularExponent) *
        materialSpecularColor * light_.specularColor;
}

vec3 shadeBlinnPhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                     vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    vec3 resAmb = materialAmbientColor * light_.ambientColor;
    vec3 resDiff = diffuse(normal, toLightDir) * materialDiffuseColor * light_.diffuseColor;
    vec3 resSpec = specularBlinnPhong(normal, toLightDir, toCameraDir, light_.specularExponent) *
        materialSpecularColor * light_.specularColor;

    return resAmb + resDiff + resSpec;
}

vec3 shadePhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    vec3 resAmb = materialAmbientColor * light_.ambientColor;
    vec3 resDiff = diffuse(normal, toLightDir) * materialDiffuseColor * light_.diffuseColor;
    vec3 resSpec = specularPhong(normal, toLightDir, toCameraDir, light_.specularExponent) *
        materialSpecularColor * light_.specularColor;
    return resAmb + resDiff + resSpec;
}

vec3 shadeBlinnPhongTwoSided(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                     vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    return shadeBlinnPhong(light_, materialAmbientColor, materialDiffuseColor, materialSpecularColor,
                           position, normal, toCameraDir);
}

vec3 shadePhongTwoSided(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    return shadePhong(light_, materialAmbientColor, materialDiffuseColor, materialSpecularColor,
                      position, normal, toCameraDir);
}

vec3 applyLighting(in LightParameters lightsource, in ShadingParameters shading, in vec3 viewDir) {
    return APPLY_LIGHTING(lightsource, shading.colors.ambient, shading.colors.diffuse,
                          shading.colors.specular, shading.worldPosition, shading.normal, viewDir);
}

#endif

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

// set reference sampling interval for opacity correction

#ifndef IVW_COMPOSITING_GLSL
#define IVW_COMPOSITING_GLSL

#define REF_SAMPLING_INTERVAL 150.0

vec4 compositeDVR(in vec4 curResult, in vec4 color, in float t, inout float tDepth,
                  in float tIncr) {
    vec4 result = curResult;

    if (tDepth == -1.0 && color.a > 0.0) tDepth = t;

    color.a = 1.0 - pow(1.0 - color.a, tIncr * REF_SAMPLING_INTERVAL);
    // front-to-back blending
    color.rgb *= color.a;
    result += (1.0 - result.a) * color;
    return result;
}

vec4 compositeMIP(in vec4 curResult, in vec4 color, in float t, inout float tDepth) {
    vec4 result = curResult;

    if (color.a > curResult.a) {
        tDepth = t;
        result = color;
    }

    return result;
}

vec4 compositeFHP(in vec4 curResult, in vec4 color, in vec3 samplePos, in float t,
                  inout float tDepth) {
    vec4 result = curResult;

    if (result == vec4(0.0) && color.a > 0.0) {
        tDepth = t;
        result = vec4(samplePos, 1.0);
    }

    return result;
}

vec4 compositeFHN(in vec4 curResult, in vec4 color, in vec3 gradient, in float t,
                  inout float tDepth) {
    vec4 result = curResult;

    if (result == vec4(0.0) && color.a > 0.0) {
        tDepth = t;
        // Note that the gradient is reversed since we define the normal of a surface as
        // the direction towards a lower intensity medium (gradient points in the inreasing direction)
        vec3 firstHitNormal = normalize(-gradient);
        result = vec4(firstHitNormal * 0.5 + 0.5, 1.0);
    }

    return result;
}

vec4 compositeFHN_VS(in vec4 curResult, in vec4 color, in vec3 gradient, in float t,
                     in CameraParameters camera, inout float tDepth) {
    vec4 result = curResult;

    if (result == vec4(0.0) && color.a > 0.0) {
        tDepth = t;
        // Note that the gradient is reversed since we define the normal of a surface as
        // the direction towards a lower intensity medium (gradient points in the inreasing direction)
        vec3 firstHitNormal = normalize(-gradient);

        // https://cloud.githubusercontent.com/assets/9251300/4753062/34392416-5ab3-11e4-9569-026a8ec9687a.png
        vec3 viewSpaceNormal = transpose(mat3(camera.worldToView)) * firstHitNormal;
        result = vec4(normalize(viewSpaceNormal.xyz) * 0.5 + 0.5, 1.0);
    }

    return result;
}

vec4 compositeFHD(in vec4 curResult, in vec4 color, in float t, inout float tDepth) {
    vec4 result = curResult;

    if (result == vec4(0.0) && color.a > 0.0) {
        tDepth = t;
        result = vec4(t, t, t, 1.0);
    }

    return result;
}

vec4 compositeISO(in vec4 curResult, in vec4 color, in float intensity, in float t,
                  inout float tDepth, in float tIncr, in float isoValue) {
    vec4 result = curResult;
    if (intensity >= isoValue - 0.01 && intensity <= isoValue + 0.01) {
        if (tDepth == -1.0) tDepth = t;
        color.a = 1.0 - pow(1.0 - color.a, tIncr * REF_SAMPLING_INTERVAL);
        // front-to-back blending
        color.rgb *= color.a;
        result += (1.0 - result.a) * color;
    }
    return result;
}

vec4 compositeISON(in vec4 curResult, in vec4 color, in float intensity, in vec3 gradient,
                   in float t, inout float tDepth, in float isoValue) {
    vec4 result = curResult;

    if (intensity >= isoValue - 0.01 && intensity <= isoValue + 0.01) {
        result = compositeFHN(curResult, color, gradient, t, tDepth);
    }

    return result;
}

#endif  // IVW_COMPOSITING_GLSL

/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#if !defined MAX_ISOVALUE_COUNT
#define MAX_ISOVALUE_COUNT 1
#endif

#if !defined INC_ISOVALUEPARAMETERS
#define INC_ISOVALUEPARAMETERS
struct IsovalueParameters {
    float values[MAX_ISOVALUE_COUNT];
    vec4 colors[MAX_ISOVALUE_COUNT];
    int size;
};
#endif

/**
 * Draws an isosurface if the given isovalue is found along the ray in between the
 * current and the previous volume sample. On return, tIncr refers to the distance
 * between the last valid isosurface and the current sampling position. That is
 * if no isosurface was found, tIncr is not modified.
 *
 * @param result           color accumulated so far during raycasting
 * @param isovalue         isovalue of isosurface to be drawn
 * @param isocolor         color of the isosurface used for blending
 * @param value            scalar values of current sampling position
 * @param previousValue    scalar values of previous sample
 * @param gradient
 * @param previousGradient
 * @param textureToWorld
 * @param lighting
 * @param rayPosition
 * @param rayDirection
 * @param toCameraDir
 * @param t
 * @param tIncr
 * @param tDepth
 * @return in case of an isosurface, result is blended with the color of the isosurface.
 *       Otherwise result is returned
 */
vec4 drawISO(in vec4 result, in float isovalue, in vec4 isocolor, in float value,
             in float previousValue, in vec3 gradient, in vec3 previousGradient,
             in mat4 textureToWorld, in LightParameters lighting, in vec3 rayPosition,
             in vec3 rayDirection, in vec3 toCameraDir, in float t, in float tIncr,
             inout float tDepth) {

    // check if the isovalue is lying in between current and previous sample
    // found isosurface if differences between current/prev sample and isovalue have different signs
    if ((isovalue - value) * (isovalue - previousValue) <= 0) {
        // apply linear interpolation between current and previous sample to obtain location of
        // isosurface
        float a = (value - isovalue) / (value - previousValue);
        // if a == 1, isosurface was already computed for previous sampling position
        if (a >= 1.0) return result;

        vec3 isopos = rayPosition - tIncr * a * rayDirection;

        ShadingParameters shadingParams = shading(isocolor.rgb);
        
        #if defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED)
        vec3 isoGradient = mix(gradient, previousGradient, a);
        shadingParams.normal = -isoGradient;

#if defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
        // backside shading only
        shadingParams.normal = -shadingParams.normal;
#elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
        // two-sided shading
        if (dot(shadingParams.normal, rayDirection) > 0.0) {
            shadingParams.normal = -shadingParams.normal;
        }
#endif

        shadingParams.worldPosition = (textureToWorld * vec4(isopos, 1.0)).xyz;
        #endif

        isocolor.rgb = applyLighting(lighting, shadingParams, toCameraDir);

        if (tDepth < 0.0) {  // blend isosurface color and adjust first-hit depth if necessary
            tDepth = t - a * tIncr;  // store depth of first hit, i.e. voxel with non-zero alpha
        }
        isocolor.rgb *= isocolor.a;  // use pre-multiplied alpha
        
        // blend isosurface color with result accumulated so far
        result += (1.0 - result.a) * isocolor;
    }

    return result;
}

vec4 drawISO(in vec4 result, in IsovalueParameters isoparams, in float value,
             in float previousValue, in vec3 gradient, in vec3 previousGradient,
             in mat4 textureToWorld, in LightParameters lighting, in vec3 rayPosition,
             in vec3 rayDirection, in vec3 toCameraDir, in float t, in float tIncr,
             inout float tDepth) {

    #if MAX_ISOVALUE_COUNT == 1
    if (isoparams.size > 0 ) {
        result = drawISO(result, isoparams.values[0], isoparams.colors[0], value, previousValue,
                         gradient, previousGradient, textureToWorld, lighting, rayPosition,
                         rayDirection, toCameraDir, t, tIncr, tDepth);
    }
    #else
    // multiple isosurfaces, need to determine order of traversal
    if (value - previousValue > 0) {
        for (int i = 0; i < isoparams.size; ++i) {
            result = drawISO(result, isoparams.values[i], isoparams.colors[i], value, previousValue,
                             gradient, previousGradient, textureToWorld, lighting, rayPosition,
                             rayDirection, toCameraDir, t, tIncr, tDepth);
        }
    } else {
        for (int i = isoparams.size; i > 0; --i) {
            result = drawISO(result, isoparams.values[i - 1], isoparams.colors[i - 1], value,
                             previousValue, gradient, previousGradient, textureToWorld, lighting,
                             rayPosition, rayDirection, toCameraDir, t, tIncr, tDepth);
        }
    }
    #endif

    return result;
}


#if !defined APPLY_LIGHTING_FUNC
#  define APPLY_LIGHTING_FUNC applyLighting
#endif // APPLY_LIGHTING_FUNC

float calcStep(in float rayLength, in vec3 direction, in float samplingRate, in vec3 dimensions) {
    float incr = min(rayLength, rayLength / (samplingRate * length(direction * dimensions)));
    float samples = ceil(rayLength / incr);
    return rayLength / samples;
}

vec3 calcCameraDir(in vec3 entryPoint, in vec3 exitPoint, in mat4 textureToWorld) {
    return normalize(
        (textureToWorld * vec4(entryPoint, 1.0) - textureToWorld * vec4(exitPoint, 1.0)).xyz);
}

layout(location = 0) out vec4 FragData0;
layout(location = 1) out vec4 PickingData;

uniform ImageParameters outportParameters;
uniform float samplingRate = 2.0;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform ImageParameters exitParameters;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;
uniform sampler2D surfaceNormal;
uniform bool useSurfaceNormals;
uniform VolumeParameters accelerateParameters;
uniform sampler3D accelerate;
uniform vec3 shift;
uniform ivec3 repeat;
uniform VolumeParameters volumeParameters;
uniform sampler3D volume;
uniform CameraParameters camera;
uniform LightParameters lighting;
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgPicking;
uniform sampler2D bgDepth;
uniform IsovalueParameters isovalues;
uniform sampler2D transferFunction;
uniform int channel = 0;

void main() {
    vec4 result = vec4(0.0);   // The accumulated color along the ray;
    vec4 picking = vec4(0.0);  // The picking color of the ray
    float rayDepth = -1.0;     // The ray depth value [0, ray length], -1 means "no" depth.
                               // Uses the same space as rayPosition. Usually used to track
                               // the depth of the "first" hit in DVR.
    float depth = 1.0;         // The image depth, from far to near [0, 1].
                               // Will be overridden by rayDepth if != -1 and then
                               // written to gl_FragDepth

    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;

    // The setup placeholder is expected to define:
    //  * entryPoint       the ray start point in the volume in texture coordinates
    //  * exitPoint        the ray exit point in the volume in texture coordinates
    //  * entryPointDepth  the image depth of the entry point
    //  * exitPointDepth   the image depth of the exit point
    //  * rayLength        the distance from the start to the exit point in texture space
    //  * rayDirection     the direction of the ray in texture space, normalized.

    vec3 entryPoint = texture(entryColor, texCoords).rgb;
    vec3 exitPoint = texture(exitColor, texCoords).rgb;
    float entryPointDepth = texture(entryDepth, texCoords).x;
    float exitPointDepth = texture(exitDepth, texCoords).x;

    vec3 sampleCount = vec3(0,0,0);
    
    // The length of the ray in texture space
    float rayLength = length(exitPoint - entryPoint);
    
    // The normalized direction of the ray
    vec3 rayDirection = normalize(exitPoint - entryPoint);
    vec4 bgColorVal = texture(bgColor, texCoords);
    vec4 bgPickingVal = texture(bgPicking, texCoords);
    depth = texture(bgDepth, texCoords).x;
    // convert to raycasting depth
    float bgRayDepth =
        rayLength * calculateTValueFromDepthValue(camera, depth, entryPointDepth, exitPointDepth);
    if (bgRayDepth <= 0) {
        if (rayDepth == -1.0 && bgColorVal.a > 0.0) rayDepth = bgRayDepth;
        if (picking.a == 0.0 && bgPickingVal.a > 0.0) picking = bgPickingVal;
        result.rgb = result.rgb + (1.0 - result.a) * bgColorVal.a * bgColorVal.rgb;
        result.a = result.a + (1.0 - result.a) * bgColorVal.a;
    }

    if (entryPoint == exitPoint) {
        FragData0 = result;
        FragData1 = vec4(sampleCount / 2000.0, 1.0);
        PickingData = picking;
        gl_FragDepth = depth;
        return;
    }

    // The step size in texture space
    float rayStep = calcStep(rayLength, rayDirection, samplingRate, volumeParameters.dimensions);

    vec3 cameraDir = calcCameraDir(entryPoint, exitPoint, volumeParameters.textureToWorld);

    // Current position along the ray
    float rayPosition = 0.5 * rayStep;
    // Current sample position in texture spcase
    vec3 samplePosition = entryPoint + rayPosition * rayDirection;

    float bigStep = calcStep(rayLength, rayDirection, 1, accelerateParameters.dimensions);
    float bigRayPosition = 0.5 * bigStep;
    vec3 bigPosition = entryPoint + bigRayPosition * rayDirection;
    //float accelerateValue = getNormalizedVoxel(accelerate, accelerateParameters, bigPosition).x;

    samplePosition *= repeat;
    samplePosition -= shift;
    vec4 volumeVoxel = getNormalizedVoxel(volume, volumeParameters, samplePosition);
    vec4 volumeVoxelPrev = volumeVoxel;
    vec3 volumeGradientPrev = vec3(0);
    vec3 volumeGradient = vec3(0);
    #if defined(GRADIENTS_ENABLED)
    volumeGradient = useSurfaceNormals ? -texture(surfaceNormal, texCoords).xyz :
        normalize(COMPUTE_GRADIENT_FOR_CHANNEL(volumeVoxel, volume, volumeParameters,
                                               samplePosition, channel));
    if (!useSurfaceNormals) {
        volumeGradient *= sign(volumeVoxel[channel] / (1.0 - volumeParameters.formatScaling) - volumeParameters.formatOffset);
    }
    #endif
    vec4 color = vec4(0);
    float accelerateValue;
    for (bigRayPosition = 0.5 * bigStep; bigRayPosition < rayLength; bigRayPosition += bigStep) {
        bigPosition = entryPoint + bigRayPosition * rayDirection;
        accelerateValue = getNormalizedVoxel(accelerate, accelerateParameters, bigPosition).x;
        if (accelerateValue != 0) {
            rayPosition = bigRayPosition;
            break;
        }
    }
    if (bigRayPosition >= rayLength) {
        rayPosition = bigRayPosition;
    }

    ShadingParameters shadingParams = defaultShadingParameters();
    color = texture(transferFunction, vec2(volumeVoxel[channel], 0.5));
    if (bgRayDepth > 0 && bgRayDepth <= rayPosition) {
        if (rayDepth == -1.0 && bgColorVal.a > 0.0) rayDepth = bgRayDepth;
        if (picking.a == 0.0 && bgPickingVal.a > 0.0) picking = bgPickingVal;
        result.rgb = result.rgb + (1.0 - result.a) * bgColorVal.a * bgColorVal.rgb;
        result.a = result.a + (1.0 - result.a) * bgColorVal.a;
    }
    if (color.a > 0) {
        shadingParams.colors = defaultMaterialColors(color.rgb);
        #if (defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED))
        shadingParams.normal = -volumeGradient;
        #if defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
        // backside shading only
        shadingParams.normal = -shadingParams.normal;
        #elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
        // two-sided shading
        if (dot(shadingParams.normal, rayDirection) > 0.0) {
            shadingParams.normal = -shadingParams.normal;
        }
        #endif
        shadingParams.worldPosition = (volumeParameters.textureToWorld * vec4(samplePosition, 1.0)).xyz;
        #endif
        color.rgb = APPLY_LIGHTING_FUNC(lighting, shadingParams, cameraDir);
    
        result = compositeDVR(result, color, rayPosition, rayDepth, rayStep);
    }

    for (rayPosition += rayStep; rayPosition < rayLength; rayPosition += rayStep) {
        sampleCount.x += 1;

        samplePosition = entryPoint + rayPosition * rayDirection;

        if (bgRayDepth > rayPosition - rayStep  && bgRayDepth <= rayPosition) {
            if (rayDepth == -1.0 && bgColorVal.a > 0.0) rayDepth = bgRayDepth;
            if (picking.a == 0.0 && bgPickingVal.a > 0.0) picking = bgPickingVal;
            result.rgb = result.rgb + (1.0 - result.a) * bgColorVal.a * bgColorVal.rgb;
            result.a = result.a + (1.0 - result.a) * bgColorVal.a;
        }


        //if (rayPosition < bigRayPosition && accelerateValue == 0.0) continue;
        //if (rayPosition < bigRayPosition && accelerateValue == 0.0) {
        //    rayPosition += 3*8*rayStep;
        //}


        sampleCount.y += 1;
        
        if (rayPosition >= bigRayPosition) {
            bigRayPosition += bigStep;
            bigPosition = entryPoint + bigRayPosition * rayDirection;
            accelerateValue = getNormalizedVoxel(accelerate, accelerateParameters, bigPosition).x;
            sampleCount.z += 1;
        }
        samplePosition *= repeat;
        samplePosition -= shift;
        volumeVoxelPrev = volumeVoxel;
        volumeVoxel = getNormalizedVoxel(volume, volumeParameters, samplePosition);
        #if defined(GRADIENTS_ENABLED)
        volumeGradientPrev = volumeGradient;
        volumeGradient = normalize(COMPUTE_GRADIENT_FOR_CHANNEL(volumeVoxel, volume, volumeParameters,
                                                             samplePosition, channel));
        volumeGradient *= sign(volumeVoxel[channel] / (1.0 - volumeParameters.formatScaling) - volumeParameters.formatOffset);
        #endif
        color = texture(transferFunction, vec2(volumeVoxel[channel], 0.5));
        if (color.a > 0) {
            shadingParams.colors = defaultMaterialColors(color.rgb);
            #if (defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED))
            shadingParams.normal = -volumeGradient;
            #if defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
            // backside shading only
            shadingParams.normal = -shadingParams.normal;
            #elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
            // two-sided shading
            if (dot(shadingParams.normal, rayDirection) > 0.0) {
                shadingParams.normal = -shadingParams.normal;
            }
            #endif
            shadingParams.worldPosition = (volumeParameters.textureToWorld * vec4(samplePosition, 1.0)).xyz;
            #endif
            color.rgb = APPLY_LIGHTING_FUNC(lighting, shadingParams, cameraDir);
        
            result = compositeDVR(result, color, rayPosition, rayDepth, rayStep);
        }

        if (result.a > 0.99) break;  // early ray termination
    }

    // composite background if lying beyond the last volume sample
    if (bgRayDepth > rayLength - rayStep * 0.5) {
        if (rayDepth == -1.0 && bgColorVal.a > 0.0) rayDepth = bgRayDepth;
        if (picking.a == 0.0 && bgPickingVal.a > 0.0) picking = bgPickingVal;
        result.rgb = result.rgb + (1.0 - result.a) * bgColorVal.a * bgColorVal.rgb;
        result.a = result.a + (1.0 - result.a) * bgColorVal.a;
    }

    depth = mix(calculateDepthValue(camera, rayDepth / rayLength, entryPointDepth, exitPointDepth),
                depth, rayDepth == -1.0);

    FragData0 = result;
    FragData1 = vec4(sampleCount / 2000.0, 1.0);
    PickingData = picking;
    gl_FragDepth = depth;
}
