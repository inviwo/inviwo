/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_ISOSURFACE_GLSL
#define IVW_ISOSURFACE_GLSL

#if !defined MAX_ISOVALUE_COUNT
#  define MAX_ISOVALUE_COUNT 5
#endif // MAX_ISOVALUE_COUNT

// need to ensure there is always at least one isovalue due to the use of the macro
// as array size in IsovalueParameters which will cause an error for size = 0.
#if MAX_ISOVALUE_COUNT < 1
#  undef MAX_ISOVALUE_COUNT
#  define MAX_ISOVALUE_COUNT 1
#endif

struct IsovalueParameters {
    float values[MAX_ISOVALUE_COUNT];
    vec4 colors[MAX_ISOVALUE_COUNT];
};

/**
 * Draws an isosurface if the given isovalue is found along the ray in between the 
 * current and the previous volume sample. On return, tIncr refers to the distance
 * between the last valid isosurface and the current sampling position. That is
 * if no isosurface was found, tIncr is not modified.
 *
 * @param curResult        color accumulated so far during raycasting
 * @param isovalue         isovalue of isosurface to be drawn
 * @param isosurfaceColor  color of the isosurface used for blending
 * @param voxel            scalar values of current sampling position
 * @param previousVoxel    scalar values of previous sample
 * @return in case of an isosurface, curResult is blended with the color of the isosurface. 
 *       Otherwise curResult is returned
 */
vec4 drawIsosurface(in vec4 curResult, in float isovalue, in vec4 isosurfaceColor, 
                    in vec4 voxel, in vec4 previousVoxel,
                    in sampler3D volume, in VolumeParameters volumeParameters, in int channel, 
                    in sampler2D transferFunction,
                    in CameraParameters camera, in LightParameters lighting,
                    in vec3 rayPosition, in vec3 rayDirection, in vec3 toCameraDir,
                    in float t, in float raySegmentLen, inout float tIncr, inout float tDepth) {
    vec4 result = curResult;

    float currentSample = voxel[channel];
    float prevSample = previousVoxel[channel];
    float sampleDelta = (currentSample - prevSample);

    // check if the isovalue is lying in between current and previous sample
    // found isosurface if differences between current/prev sample and isovalue have different signs
    if ((isovalue - currentSample) * (isovalue - prevSample) <= 0) {
        // apply linear interpolation between current and previous sample to obtain location of isosurface
        float a = (currentSample - isovalue) / sampleDelta;
        // if a == 1, isosurface was already computed for previous sampling position
        if (a >= 1.0) return result;
        
        // adjust length of remaining ray segment
        tIncr = a * raySegmentLen;

        vec3 isopos = rayPosition - raySegmentLen * a * rayDirection;

        vec4 isocolor = isosurfaceColor;
#if defined(SHADING_ENABLED)
        vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, isopos, channel);
        gradient = normalize(gradient);

        // two-sided lighting
        if (dot(gradient, rayDirection) <= 0) {
            gradient = -gradient;
        }

        vec3 isoposWorld = (volumeParameters.textureToWorld * vec4(isopos, 1.0)).xyz;
        isocolor.rgb = APPLY_LIGHTING(lighting, isocolor.rgb, isocolor.rgb, vec3(1.0),
                           isoposWorld, -gradient, toCameraDir);
#endif // SHADING_ENABLED

#if defined(INCLUDE_DVR)
        // apply compositing of volumetric media from last sampling position up till isosurface
        vec4 voxelColor = APPLY_CHANNEL_CLASSIFICATION(transferFunction, vec4(isovalue), channel);
        if (voxelColor.a > 0) {
#if defined(SHADING_ENABLED)
            voxelColor.rgb = APPLY_LIGHTING(lighting, voxelColor.rgb, voxelColor.rgb, vec3(1.0),
                                       isoposWorld, -gradient, toCameraDir);
#endif // SHADING_ENABLED

            result = APPLY_COMPOSITING(result, voxelColor, isopos, vec4(isovalue), gradient, camera,
                                       isovalue, t - tIncr, tDepth, raySegmentLen - tIncr);
        }
#endif // INCLUDE_DVR

        // blend isosurface color and adjust first-hit depth if necessary
        if (tDepth < 0.0) {
            // store depth of first hit, i.e. voxel with non-zero alpha
             tDepth = t - tIncr;   
        }
        isocolor.rgb *= isocolor.a; // use pre-multiplied alpha
        // blend isosurface color with result accumulated so far
        result += (1.0 - result.a) * isocolor;
    }

    return result;
}


vec4 drawIsosurfaces(in vec4 curResult, in IsovalueParameters isoparams, 
                     in vec4 voxel, in vec4 previousVoxel,
                     in sampler3D volume, in VolumeParameters volumeParameters, in int channel, 
                     in sampler2D transferFunction,
                     in CameraParameters camera, in LightParameters lighting,
                     in vec3 rayPosition, in vec3 rayDirection, in vec3 toCameraDir,
                     in float t, inout float tIncr, inout float tDepth) {

    // in case of zero no isovalues return current color
    vec4 result = curResult;
    float raySegmentLen = tIncr;

#if defined(ISOSURFACE_ENABLED)

#if MAX_ISOVALUE_COUNT == 1
    result = drawIsosurface(result, isoparams.values[0], isoparams.colors[0],
                            voxel, previousVoxel, volume, volumeParameters, channel, transferFunction,
                            camera, lighting, rayPosition, rayDirection, toCameraDir, t, raySegmentLen, tIncr, tDepth);
#else // MAX_ISOVALUE_COUNT
    // multiple isosurfaces, need to determine order of traversal
    if (voxel[channel] - previousVoxel[channel] > 0) {
        for (int i = 0; i < MAX_ISOVALUE_COUNT; ++i) {
            result = drawIsosurface(result, isoparams.values[i], isoparams.colors[i],
                voxel, previousVoxel, volume, volumeParameters, channel, transferFunction,
                camera, lighting, rayPosition, rayDirection, toCameraDir, t, raySegmentLen, tIncr, tDepth);
        }
    } else {
        for (int i = MAX_ISOVALUE_COUNT; i > 0; --i) {
            result = drawIsosurface(result, isoparams.values[i - 1], isoparams.colors[i - 1],
                voxel, previousVoxel, volume, volumeParameters, channel, transferFunction,
                camera, lighting, rayPosition, rayDirection, toCameraDir, t, raySegmentLen, tIncr, tDepth);
        }
    }
#endif // MAX_ISOVALUE_COUNT
#endif // ISOSURFACE_ENABLED

    return result;
}

#endif // IVW_ISOSURFACE_GLSL
