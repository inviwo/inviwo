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

#ifndef IVW_STRUCTS_GLSL
#define IVW_STRUCTS_GLSL

struct CameraParameters {
   mat4 worldToView; // Equivalent to view
   mat4 viewToWorld; // Equivalent to viewInverse
   mat4 worldToClip; // Equivalent to viewProjection
   mat4 clipToWorld; // Equivalent to viewProjectionInverse
   vec3 position;
   float nearPlane;  // zNear
   float farPlane;   // zFar
};
// Convenience functions to retrieve camera parameters
/* lookAt(vec3 eye, vec3 center, vec3 up) {
    vec3 cameraDir(normalize(center - eye));
    vec3 right(normalize(cross(cameraDir, up)));
    up = (cross(right, cameraDir));
    vec3 lookFrom(dot(right, eye), dot(up, eye), -dot(cameraDir, eye));
    [  right[0], up[0], -cameraDir[0], -lookFrom[0] ] 
    [  right[1], up[1], -cameraDir[1], -lookFrom[1] ] 
    [  right[2], up[2], -cameraDir[2], -lookFrom[2] ] 
    [         0,     0,         0,       1     ] */

// Does this work??
vec3 right(in CameraParameters camera) { return camera.worldToView[0].xyz; }
vec3 up(in CameraParameters camera) { return camera.worldToView[1].xyz; }
vec3 direction(in CameraParameters camera) { return -camera.worldToView[2].xyz; }

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
    mat4 textureToIndex;                   // Transform from [0 1] to [-0.5 dim-0.5]
    mat4 indexToTexture;                   // Transform from [-0.5 dim-0.5] to [0 1]
    mat3 textureSpaceGradientSpacing;      // Maximum possible distance to go without ending up 
										   // outside of a voxel (half of minimum voxel spacing 
										   // for volumes with orthogonal basis)
    vec3 dimensions;		               // Number of voxels (dim) per axis 
    vec3 reciprocalDimensions;             // 1 over the number of voxels
    float worldSpaceGradientSpacing;       // Spacing between gradient samples in world space 
    float formatScaling;                   // Map value from data range [min,max] to [0,1]
    float formatOffset;
    float signedFormatScaling;             // Map value from data range [min,max] to [-1,1]
    float signedFormatOffset;
};

struct ImageParameters {
    mat3 dataToModel;
    mat3 modelToData;
    mat3 dataToWorld;
    mat3 worldToData;
    mat3 modelToWorld;
    mat3 worldToModel;
    mat3 worldToTexture;
    mat3 textureToWorld;
    mat3 textureToIndex;  // Transform from [0 1] to [-0.5 dim-0.5]
    mat3 indexToTexture; // Transform from [-0.5 dim-0.5] to [0 1]
    vec2 dimensions;
    vec2 reciprocalDimensions;
};

struct LightParameters {
    vec3 position; 
    vec3 ambientColor;
    vec3 diffuseColor; 
    vec3 specularColor;
    float specularExponent;
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

#endif // IVW_STRUCTS_GLSL