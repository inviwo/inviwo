/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

// How should the stuff be rendered? (Debugging options)
#define ABUFFER_DISPNUMFRAGMENTS 0
#define ABUFFER_RESOLVE_USE_SORTING 1

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgDepth;
uniform vec2 reciprocalDimensions;
#endif  // BACKGROUND_AVAILABLE

struct RaycastingInfo {
    vec3 samplePosition;
    bool isActive;
};
RaycastingInfo raycastingInfos[8] = RaycastingInfo[8](
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false),
	RaycastingInfo(vec3(0.0), false));
uniform CameraParameters camera;
uniform float samplingRate;
uniform sampler3D volumeSamplers[8];
uniform sampler2D tfSamplers[8];

uniform VolumeParameters volumeParameters[8];



// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;

// Computes only the number of fragments
int getFragmentCount(uint pixelIdx);

// Keeps only closest fragment
vec4 resolveClosest(uint idx);

vec3 screenToData(VolumeParameters volume, float depth, vec2 fragCoords) {
    return vec3(volume.worldToData * camera.clipToWorld * vec4(fragCoords, depth, 1.0));
}

vec4 myRaycast(vec4 color, int volumeIndex, vec3 entryData, vec3 exitData) {
    vec4 res = color;
    float t = 0.0;
    vec3 rayDir = exitData - entryData;
    float tEnd = length(exitData - entryData);
    float tIncr = min(
        tEnd, tEnd / (samplingRate * length(rayDir * volumeParameters[volumeIndex].dimensions)));
    while(t < tEnd) {
        vec3 samplePos = entryData * rayDir * t;
        float values = texture(volumeSamplers[volumeIndex], samplePos).r;
        t += tIncr;
        //applyTF()
        res += vec4(tIncr);
    }
    
    return vec4(1.0, 0.0, 1.0, 1.0);
}

void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    vec2 screenPos = gl_FragCoord.xy / vec2(AbufferParams.screenWidth, AbufferParams.screenHeight);
    if (coords.x < 0 || coords.y < 0 || coords.x >= AbufferParams.screenWidth ||
        coords.y >= AbufferParams.screenHeight) {
        discard;
    }

    uint pixelIdx = getPixelLink(coords);

    if (pixelIdx > 0) {
        float backgroundDepth = 1.0;
#ifdef BACKGROUND_AVAILABLE
        // Assume the camera used to render the background has the same near and far plane,
        // so we can directly compare depths.
        vec2 texCoord = (gl_FragCoord.xy + 0.5) * reciprocalDimensions;
        backgroundDepth = texture(bgDepth, texCoord).x;
#endif  // BACKGROUND_AVAILABLE

        // Initialize fragments
        abufferMeshPixel unpackedMeshFragment = abufferMeshPixel(0, 1.0, vec4(0.0, 1.0, 1.0, 1.0));
        abufferVolumePixel unpackedVolumeFragment = abufferVolumePixel(0, 1.0, vec3(0.0), 0);
        // front-to-back shading
        vec4 color = vec4(0);
        uint lastPtr = 0;
        vec4 nextFragment = selectionSortNext(pixelIdx, 0.0, lastPtr);
        int type = getPixelDataType(nextFragment); // Get type of closest fragment
        
        // Store two depths to raycast between
        float depth = 0.0;
        float nextDepth = 0.0;

        // Unpack closest fragment + init depths
        if(type == 0){
            unpackedMeshFragment = uncompressMeshPixelData(nextFragment);
            gl_FragDepth = min(backgroundDepth, unpackedMeshFragment.depth);
            depth = unpackedMeshFragment.depth;
        } else if(type == 1) {
            unpackedVolumeFragment = uncompressVolumePixelData(nextFragment);
            gl_FragDepth = min(backgroundDepth, unpackedVolumeFragment.depth);
            depth = unpackedVolumeFragment.depth;
        }

        while (depth >= 0 && depth <= backgroundDepth) {
            if(type == 0){ // Shade mesh + find next fragment
                vec4 c = unpackedMeshFragment.color;
                color.rgb = color.rgb + (1 - color.a) * c.a * c.rgb;
                color.a = color.a + (1 - color.a) * c.a;
                nextFragment = selectionSortNext(pixelIdx, unpackedMeshFragment.depth, lastPtr);
            } else if(type == 1) {
                raycastingInfos[unpackedVolumeFragment.id].isActive = !raycastingInfos[unpackedVolumeFragment.id].isActive; 
                raycastingInfos[unpackedVolumeFragment.id].samplePosition = unpackedVolumeFragment.position;
                nextFragment = selectionSortNext(pixelIdx, unpackedVolumeFragment.depth, lastPtr);
            }

            // Check depth of next fragment
            type = getPixelDataType(nextFragment);
            if(type == 0) {
                unpackedMeshFragment = uncompressMeshPixelData(nextFragment);
                nextDepth = unpackedMeshFragment.depth;
            } else if(type == 1) {
                unpackedVolumeFragment = uncompressVolumePixelData(nextFragment);
                nextDepth = unpackedVolumeFragment.depth;
            }

            //
            for(int volumeIndex = 0; volumeIndex < 8; ++volumeIndex) {
                if(raycastingInfos[volumeIndex].isActive) {
                    vec3 viewDir = normalize(screenToData(volumeParameters[volumeIndex], nextDepth, screenPos) - screenToData(volumeParameters[volumeIndex], depth, screenPos));
                    vec3 entryPos = screenToData(volumeParameters[volumeIndex], depth, screenPos);
                    vec3 exitPos = screenToData(volumeParameters[volumeIndex], nextDepth, screenPos);
                    color += myRaycast(color, volumeIndex, entryPos, exitPos); // depth + next = 0-1
                }
            }
            depth = nextDepth; // Prevent infinite loop
        }
        FragData0 = color;
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
    } else {  // no pixel found
        FragData0 = vec4(0.0);
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
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
    // Output final color for the frame buffer
    return minFrag;
}