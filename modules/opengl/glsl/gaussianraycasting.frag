/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"
#include "utils/sampler3d.glsl"

#include "utils/classification.glsl"
#include "utils/compositing.glsl"
#include "utils/depth.glsl"
#include "utils/gradients.glsl"
#include "utils/shading.glsl"
#include "utils/raycastgeometry.glsl"

#include "utils/isosurface.glsl"

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;
uniform sampler2D layerPort;
uniform sampler2D transferFunction;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform sampler2D entryPicking;
uniform sampler2D entryNormal;

uniform ImageParameters exitParameters;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;

uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgPicking;
uniform sampler2D bgDepth;

uniform ImageParameters outportParameters;

uniform LightParameters lighting;
uniform CameraParameters camera;
uniform VolumeIndicatorParameters positionindicator;
uniform RaycastingParameters raycaster;
uniform IsovalueParameters isovalues;

uniform bool useNormals = false;

uniform int channel;

uniform float sigma;

#define M_PI 3.14159
#define ERT_THRESHOLD 0.99  // threshold for early ray termination

#if (!defined(INCLUDE_DVR) && !defined(INCLUDE_ISOSURFACES))
#  define INCLUDE_DVR
#endif

vec4 centerPoints[500];
int count;

int load() {
    ivec2 texCoords = textureSize(layerPort, 0);

    int c = 0;
    for(int i = 0; i < texCoords.x; ++i) {
        for(int j = 0; j < texCoords.y; ++j) {
            vec2 cord = vec2((float(i) + 0.5)/texCoords.x , (float(j) + 0.5)/texCoords.y);
            vec4 point = texture(layerPort, cord);
            centerPoints[c] = point;
            ++c;
        }
    }

    return c;
}


vec4 sumFunction(float sigma, vec3 samplePos, float tIncr, int count){
    vec4 result = vec4(0.0);
    float sum = 0;
    for(int i = 0; i < count; ++i) {
        
        float s = sigma*centerPoints[i].w;
        vec3 dr = centerPoints[i].xyz - samplePos;
        float r2 = dr.x * dr.x + dr.y * dr.y + dr.z * dr.z;
        float A = 1.0 / (s*sqrt(2*M_PI));
        float B = 0.5*r2/(s*s);

        float v = A*exp(-B*r2);
        vec3 grad = -2*B*dr*v;
        

        result += vec4(grad, v);
        
        /*float s = centerPoints[i].w;
        vec3 dp = (centerPoints[i].xyz - samplePos);
        float r2 = (dp.x*dp.x + dp.y*dp.y + dp.z*dp.z);
        sum += 1.0/(s*sqrt(2*M_PI))*exp(-0.5*r2/(s*s))*tIncr*100*/;


    }
    
    return result;
}

vec4 rayTraversal(vec3 entryPoint, vec3 exitPoint, vec2 texCoords, float backgroundDepth, vec3 entryNormal) {
    vec4 result = vec4(0.0);
    vec3 rayDirection = exitPoint - entryPoint;
    float tEnd = length(rayDirection);
    float tIncr = min(
        tEnd, tEnd / (raycaster.samplingRate * length(rayDirection * volumeParameters.dimensions)));
    float samples = ceil(tEnd / tIncr);
    tIncr = tEnd / samples;
    //tIncr = 0.1;
    float t = 0.5f * tIncr;
    rayDirection = normalize(rayDirection);
    float tDepth = -1.0;
    vec4 color;
    vec4 voxel;
    vec3 samplePos;
    
    ShadingParameters shadingParams = defaultShadingParameters();
    vec3 toCameraDir = normalize((volumeParameters.textureToWorld * vec4(entryPoint, 1.0) -
                                  volumeParameters.textureToWorld * vec4(exitPoint, 1.0))
                                     .xyz);

    vec4 backgroundColor = vec4(0);
    float bgTDepth = -1;
#ifdef BACKGROUND_AVAILABLE
    backgroundColor = texture(bgColor, texCoords);
    // convert to raycasting depth
    bgTDepth = tEnd * calculateTValueFromDepthValue(
        camera, backgroundDepth, texture(entryDepth, texCoords).x, texture(exitDepth, texCoords).x);        

    if (bgTDepth < t) {
        result = backgroundColor;
    }
#endif // BACKGROUND_AVAILABLE
    
    bool first = true;
    
    
    
    
    while (t < tEnd) {
    
        samplePos = entryPoint + t * rayDirection;       

#if defined(BACKGROUND_AVAILABLE)
        result = DRAW_BACKGROUND(result, t, tIncr, backgroundColor, bgTDepth, tDepth);
#endif // BACKGROUND_AVAILABLE
        
        vec4 res = sumFunction(sigma, samplePos, tIncr, count);
        //sum = res.w;

        vec3 gradient = normalize(res.xyz);

        color = applyTF(transferFunction, res.w);

        ShadingParameters shadingParams = shading(color.rgb, gradient, 
                                    (volumeParameters.textureToWorld * vec4(samplePos, 1.0)).xyz);
        color.rgb = applyLighting(lighting, shadingParams, toCameraDir);   

        if (color.a > 0) {
            result = compositeDVR(result, color, t, tDepth, tIncr);
            
        }


        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
#if defined(ISOSURFACE_ENABLED) && defined(INCLUDE_ISOSURFACES)
            // make sure that tIncr has the correct length since drawIsoSurface will modify it
            tIncr = tEnd / samples;
#endif // ISOSURFACE_ENABLED
            t += tIncr;
        }
        first = false;
    


    }//end sampler loop

    // composite background if lying beyond the last volume sample, which is located at tEnd - tIncr*0.5
    if (bgTDepth > tEnd - tIncr * 0.5) {
        result =
            DRAW_BACKGROUND(result, bgTDepth, tIncr * 0.5, backgroundColor, bgTDepth, tDepth);
    }

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera, tDepth / tEnd, texture(entryDepth, texCoords).x,
                                     texture(exitDepth, texCoords).x);

    } else {
        tDepth = 1.0;
    }

    gl_FragDepth = min(backgroundDepth, tDepth);

    return result;
}

void main() {
    count = load();
    
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec3 entryPoint = texture(entryColor, texCoords).rgb;
    vec3 exitPoint = texture(exitColor, texCoords).rgb;

    vec4 color = vec4(0);

    float backgroundDepth = 1;
#ifdef BACKGROUND_AVAILABLE
    color = texture(bgColor, texCoords);
    gl_FragDepth = backgroundDepth = texture(bgDepth, texCoords).x;
    PickingData = texture(bgPicking, texCoords);
#else // BACKGROUND_AVAILABLE
    PickingData = vec4(0);
    if (entryPoint == exitPoint) {
        discard;
    }
#endif // BACKGROUND_AVAILABLE

    vec3 normal = useNormals ? texture(entryNormal, texCoords).xyz : vec3(0,0,0);
    if (entryPoint != exitPoint) {
        color = rayTraversal(entryPoint, exitPoint, texCoords, backgroundDepth, normal);
    }
    FragData0 = color;
}
