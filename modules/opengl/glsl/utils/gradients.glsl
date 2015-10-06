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

#ifndef IVW_GRADIENTS_GLSL
#define IVW_GRADIENTS_GLSL

// Compute gradient for 1 channel.

// Compute world space gradient using forward difference: f' = ( f(x+h)-f(x) ) / h
vec3 gradientForwardDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) forward differences
    // Value at f(x+h)
    vec3 fDs;
    fDs.x = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0])[channel];
    fDs.y = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1])[channel];
    fDs.z = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2])[channel];
    // Note that this computation is performed in world space
    // f' = ( f(x+h)-f(x) ) / volumeParams.worldSpaceGradientSpacing
    return (fDs-intensity[channel])/(volumeParams.worldSpaceGradientSpacing);
}

// Compute world space gradient using central difference: f' = ( f(x+h)-f(x-h) ) / 2*h
vec3 gradientCentralDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) central differences
    vec3 cDs;
    // Value at f(x+h)
    cDs.x = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0])[channel];
    cDs.y = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1])[channel];
    cDs.z = getNormalizedVoxel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2])[channel];
    // Value at f(x-h)
    cDs.x = cDs.x - getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0])[channel];
    cDs.y = cDs.y - getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1])[channel];
    cDs.z = cDs.z - getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2])[channel];
    // Note that this computation is performed in world space
    // f' = ( f(x+h)-f(x-h) ) / 2*volumeParams.worldSpaceGradientSpacing
    return (cDs)/(2.0*volumeParams.worldSpaceGradientSpacing);
}

// Compute world space gradient using backward difference: f' = ( f(x)-f(x-h) ) / h
vec3 gradientBackwardDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) backward differences
    // Value at f(x-h)
    vec3 fDs;
    fDs.x = getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0])[channel];
    fDs.y = getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1])[channel];
    fDs.z = getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2])[channel];
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
    cDs.x = 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos +volumeParams.textureSpaceGradientSpacing[0])[channel];
    cDs.y = 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos +volumeParams.textureSpaceGradientSpacing[1])[channel];
    cDs.z = 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos +volumeParams.textureSpaceGradientSpacing[2])[channel];
    // Value at 8.f(x-h)
    cDs.x = cDs.x - 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0])[channel];
    cDs.y = cDs.y - 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1])[channel];
    cDs.z = cDs.z - 8.0 * getNormalizedVoxel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2])[channel];
    // Value at -f(x+2h)
    cDs.x = cDs.x - getNormalizedVoxel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[0])[channel];
    cDs.y = cDs.y - getNormalizedVoxel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[1])[channel];
    cDs.z = cDs.z - getNormalizedVoxel(volume, volumeParams, samplePos + 2.0*volumeParams.textureSpaceGradientSpacing[2])[channel];
    // Value at f(x+2h)
    cDs.x = cDs.x + getNormalizedVoxel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[0])[channel];
    cDs.y = cDs.y + getNormalizedVoxel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[1])[channel];
    cDs.z = cDs.z + getNormalizedVoxel(volume, volumeParams, samplePos - 2.0*volumeParams.textureSpaceGradientSpacing[2])[channel];
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
    return transpose(fDs - mat3x4(intensity,intensity,intensity)) / volumeParams.worldSpaceGradientSpacing;
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
    return transpose(cDs) / (2.0*volumeParams.worldSpaceGradientSpacing);
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
    return transpose(mat3x4(intensity,intensity,intensity)-fDs) / volumeParams.worldSpaceGradientSpacing;
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
    return transpose(cDs)/(12.0*volumeParams.worldSpaceGradientSpacing);
}

// Use pre-computed gradients stored in xyz of the current sample. The gradient will be transformed from texture space
// to world space by using the textureToWorldNormalMatrix, i.e. the inverse transposed textureToWorld matrix.
vec3 gradientPrecomputedXYZ(vec4 intensity, VolumeParameters volumeParams) {
    vec3 gradient = intensity.xyz;
    return (volumeParams.textureToWorldNormalMatrix * vec4(gradient, 0.0)).xyz;
}

// Use pre-computed gradients stored in yzw of the current sample. The gradient will be transformed from texture space
// to world space by using the textureToWorldNormalMatrix, i.e. the inverse transposed textureToWorld matrix.
vec3 gradientPrecomputedYZW(vec4 intensity, VolumeParameters volumeParams) {
    vec3 gradient = intensity.yzw;
    return (volumeParams.textureToWorldNormalMatrix * vec4(gradient, 0.0)).xyz;
}

#endif // IVW_GRADIENTS_GLSL

