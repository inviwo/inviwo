/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

/**
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0
 * Copyright Cyril Crassin, July 2010
 *
 * Edited by the Inviwo Foundation.
 **/

// need extensions added from C++
// GL_NV_gpu_shader5, GL_EXT_shader_image_load_store, GL_NV_shader_buffer_load,
// GL_EXT_bindable_uniform

#include "oit/myabufferlinkedlist.glsl"
#include "oit/mysort.glsl"
#include "utils/structs.glsl"
#include "utils/classification.glsl"
#include "utils/depth.glsl"
#include "utils/sampler3d.glsl"

#if !defined(REF_SAMPLING_INTERVAL)
#define REF_SAMPLING_INTERVAL 150.0
#endif

#define ERT_THRESHOLD 0.99  // threshold for early ray termination

// How should the stuff be rendered? (Debugging options)
#define ABUFFER_DISPNUMFRAGMENTS 0
#define ABUFFER_FIRSTHITONLY 0
#define ABUFFER_RESOLVE_USE_SORTING 1

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgDepth;
uniform vec2 reciprocalDimensions;
#endif  // BACKGROUND_AVAILABLE

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;


uniform CameraParameters camera;
uniform float samplingDistance; // distance between two volume samples in world space

// NOTE: when adjusting the number of array samples ensure that _all_ uniform arrays and the 
// raycastingInfos have the same size. The switch-case in the raycasting loop _must_ cover all
// samplers. Otherwise the volumes are not considered.
//
// ```
// switch (volumeIndex) {
//     case 0:
//         scalar = sampleVolume(volumeSamplers[0], volumeIndex, samplePosWorld);
//         break;
//     ...
//     case n:
//         scalar = sampleVolume(volumeSamplers[n], volumeIndex, samplePosWorld);
//         break;
//     default:
//         break;
// }
// ```

uniform sampler3D volumeSamplers[4];
uniform sampler2D tfSamplers[4];
uniform VolumeParameters volumeParameters[4];
uniform int volumeChannels[4];
uniform float opacityScaling[4] = float[4](1, 1, 1, 1);
uniform LightParameters lighting[4];

struct RaycastingInfo {
    // TODO: needed for illumination calculations or similar?
    vec3 rayDir; // in data space
    bool isActive;
};

RaycastingInfo raycastingInfos[4];

// Computes only the number of fragments
int getFragmentCount(uint pixelIdx);

// Returns the closest fragment of the fragment list
vec4 resolveClosest(uint idx);

// Transform the non-linear \p fragDepth at a given normalized screen position \p screenPos
// from [0,1] to world coordinates
// @return corresponding linear depth in world space
float depthToWorld(in CameraParameters camera, in vec2 screenPos, in float fragDepth) {
    vec4 worldpos = camera.clipToWorld * vec4(vec3(screenPos, fragDepth) * 2.0 - 1.0, 1);
    worldpos /= worldpos.w;
    float depthWorld = distance(camera.position, worldpos.xyz); 
    return depthWorld;
}

// Sample the given volume sampler \p volume of volume \p volumeIndex at \p posWorld in world space
// @return normalized scalar value [0,1]
float sampleVolume(in sampler3D volume, in uint volumeIndex, in vec3 posWorld) {
    vec3 samplePosData = (volumeParameters[volumeIndex].worldToData * vec4(posWorld, 1)).xyz;
    float scalar = texture(volume, samplePosData)[volumeChannels[volumeIndex]];
    // normalize scalar value to [0,1]
    scalar = (scalar  + volumeParameters[volumeIndex].formatOffset) * 
        (1.0 - volumeParameters[volumeIndex].formatScaling);
    return scalar;
}

// Compute the absorption along distance \p tIncr according to the volume rendering equation. The 
// \p opacityScaling factor is used to scale the extinction to account for differently sized datasets.
// @return opacity for ray segment of length \p tIncr
float absorption(in uint volumeIndex, in float opacity, in float tIncr) {
    return 1.0 - pow(1.0 - opacity, tIncr * REF_SAMPLING_INTERVAL * opacityScaling[volumeIndex]);
}

void initRaycastingInfo() {
    for (int i = 0; i < 4; ++i) {
        // determine the ray direction in data space
        mat4 clipToData = volumeParameters[i].worldToData * camera.clipToWorld;
        vec4 posData = clipToData * vec4(gl_FragCoord.xy, 1.0, 1.0);
        posData /= posData.w;

        vec4 camposData = volumeParameters[i].worldToData * vec4(camera.position, 1.0);
        camposData /= camposData.w;

        raycastingInfos[i].rayDir = normalize(vec3(camposData) - vec3(camposData));
        raycastingInfos[i].isActive = false;
    }
}

void main() {
    initRaycastingInfo();
    
    const ivec2 coords = ivec2(gl_FragCoord.xy);
    const vec2 screenPos = gl_FragCoord.xy / vec2(AbufferParams.screenWidth, AbufferParams.screenHeight);
    if (coords.x < 0 || coords.y < 0 || coords.x >= AbufferParams.screenWidth ||
        coords.y >= AbufferParams.screenHeight) {
        discard;
    }

    const float tIncr = samplingDistance;

    vec4 p = camera.clipToWorld * vec4(vec3(screenPos, 0.5) * 2.0 - 1.0, 1);
    const vec3 rayDir = normalize(p.xyz/p.w - camera.position);

    uint pixelIdx = getPixelLink(coords);
    if (pixelIdx > 0) {

#if ABUFFER_DISPNUMFRAGMENTS == 1
        FragData0 = vec4(vec3(getFragmentCount(pixelIdx) / float(ABUFFER_SIZE)), 1);
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
        return;
#elif ABUFFER_FIRSTHITONLY == 1
        // get only first hit that is the closest one
        vec4 p = resolveClosest(pixelIdx);
        int ptype = getPixelDataType(p);

        if(ptype == 0){
            abufferMeshPixel meshfrag = uncompressMeshPixelData(p);
            FragData0 = meshfrag.color;
            gl_FragDepth = meshfrag.depth;
        } else if(ptype == 1) {
            abufferVolumePixel volfrag = uncompressVolumePixelData(p);
            FragData0 = vec4(volfrag.position, 1);
            gl_FragDepth = volfrag.depth;
        }
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
        return;
#endif

        float backgroundDepth = 1.0;
#ifdef BACKGROUND_AVAILABLE
        vec2 texCoord = (gl_FragCoord.xy + 0.5) * reciprocalDimensions;
        backgroundDepth = texture(bgDepth, texCoord).x;
#endif  // BACKGROUND_AVAILABLE

        vec4 dstColor = vec4(0);
        uint lastPtr = 0;
        vec4 nextFragment = selectionSortNext(pixelIdx, 0.0, lastPtr);
        int fragType = getPixelDataType(nextFragment);
        
        // non-linear depth in normalized device coords [0,1]
        float depth = 0.0;

        abufferMeshPixel meshFrag = abufferMeshPixel(0, 1.0, vec4(0.0, 1.0, 1.0, 1.0));
        abufferVolumePixel volumeFrag = abufferVolumePixel(0, 1.0, vec3(0.0), 0);

        if (fragType == 0){
            meshFrag = uncompressMeshPixelData(nextFragment);
            gl_FragDepth = min(backgroundDepth, meshFrag.depth);
            depth = meshFrag.depth;
        } else if (fragType == 1) {
            volumeFrag = uncompressVolumePixelData(nextFragment);
            gl_FragDepth = min(backgroundDepth, volumeFrag.depth);
            depth = volumeFrag.depth;
        }

        while (depth >= 0 && depth <= backgroundDepth) {
            if (fragType == 0) {
                vec4 color = meshFrag.color;
                color.rgb *= color.a;
                dstColor += (1.0 - dstColor.a) * color;
            } else if (fragType == 1) {
                raycastingInfos[volumeFrag.id].isActive = !raycastingInfos[volumeFrag.id].isActive; 
            }

            // determine depth of next fragment to define the current ray segment for raycasting
            nextFragment = selectionSortNext(pixelIdx, depth, lastPtr);
            int nextFragType = getPixelDataType(nextFragment);
            float nextFragDepth = depth;
            if (nextFragType == 0) {
                meshFrag = uncompressMeshPixelData(nextFragment);
                nextFragDepth = meshFrag.depth;
            } else if(nextFragType == 1) {
                volumeFrag = uncompressVolumePixelData(nextFragment);
                nextFragDepth = volumeFrag.depth;
            }
            fragType = nextFragType;

            // transform depth to world space, align sampling positions with camera
            // NOTE: alignment leads to artifacts during camera transformations due to 
            // camera-fixated sampling positions. Especially noticeable during coarse sampling.
            float tStart = ceil(depthToWorld(camera, screenPos, depth) / tIncr) * tIncr;
            // tStart = depthToWorld(camera, screenPos, depth);
            float tEnd = depthToWorld(camera, screenPos, nextFragDepth);

            // integrate ray segments of all volumes between current depth and next fragment in world space
            float t = tStart;
            while (t < tEnd) {
                const vec3 samplePosWorld = camera.position + rayDir * t;

                for(int volumeIndex = 0; volumeIndex < 2; ++volumeIndex) {
                    if(raycastingInfos[volumeIndex].isActive) {
                        float scalar = 0.0;
                        // texture sampling cannot be moved into a separate function due to the texture lookup 
                        // depending on volumeIndex. 
                        // Arrays of samplers can only be accessed by compile-time integral constant expressions.
                        // see https://www.khronos.org/opengl/wiki/Data_Type_(GLSL)#Opaque_arrays
                        switch (volumeIndex) {
                            case 0:
                                scalar = sampleVolume(volumeSamplers[0], volumeIndex, samplePosWorld);
                                break;
                            case 1:
                                scalar = sampleVolume(volumeSamplers[1], volumeIndex, samplePosWorld);
                                break;
                            case 2:
                                scalar = sampleVolume(volumeSamplers[2], volumeIndex, samplePosWorld);
                                break;
                            case 3:
                                scalar = sampleVolume(volumeSamplers[3], volumeIndex, samplePosWorld);
                                break;
                            default:
                                break;
                        }

                        vec4 color = applyTF(tfSamplers[volumeIndex], scalar);

                        // volume integration along current segment
                        color.a = absorption(volumeIndex, color.a, tIncr);
                        // front-to-back blending
                        color.rgb *= color.a;
                        dstColor += (1.0 - dstColor.a) * color;
                    }
                }
                t += tIncr;
            }
           
            if (dstColor.a >= ERT_THRESHOLD) break;

            depth = nextFragDepth;
        }
        FragData0 = dstColor;
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
    } else {  // no pixel found
#if ABUFFER_DISPNUMFRAGMENTS == 0
        // If no fragment, write nothing
        discard;
#else
        FragData0 = vec4(0.0);
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
#endif
    }
}

int getFragmentCount(uint pixelIdx) {
    int counter = 0;
    while (pixelIdx != 0 && counter < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);
        counter++;
        pixelIdx = floatBitsToUint(val.x);
    }
    return counter;
}

vec4 resolveClosest(uint pixelIdx) {
    // Search smallest z
    vec4 minFrag = vec4(0.0f, 1000000.0f, 1.0f, uintBitsToFloat(1024 * 1023));
    int ip = 0;

    while (pixelIdx != 0 && ip < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);

        if (val.y < minFrag.y) {
            minFrag = val;
        }

        pixelIdx = floatBitsToUint(val.x);

        ip++;
    }
    return minFrag;
}