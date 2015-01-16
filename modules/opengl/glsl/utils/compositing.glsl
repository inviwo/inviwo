/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

// set reference sampling interval for opacity correction
#define REF_SAMPLING_INTERVAL 150.0

vec4 compositeDVR(in vec4 curResult, in vec4 color, in float t, inout float tDepth,
                  in float tIncr) {
    vec4 result = curResult;

    if (tDepth == -1.0 && color.a > 0.0) tDepth = t;

    color.a = 1.0 - pow(1.0 - color.a, tIncr * REF_SAMPLING_INTERVAL);
    result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
    result.a = result.a + (1.0 - result.a) * color.a;
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
        result = vec4(normalize(gradient) * 0.5 + 0.5, 1.0);
    }

    return result;
}

vec4 compositeFHN_VS(in vec4 curResult, in vec4 color, in vec3 gradient, in float t,
                     in CameraParameters camera, inout float tDepth) {
    vec4 result = curResult;

    if (result == vec4(0.0) && color.a > 0.0) {
        tDepth = t;
        vec4 fh_normal = vec4(normalize(gradient), 0.0);
        // TODO: This transformation is incorrect
        // should be transpose(mat3(viewToWorld))* fh_normal
        // https://cloud.githubusercontent.com/assets/9251300/4753062/34392416-5ab3-11e4-9569-026a8ec9687a.png
        vec4 transformed_normal = camera.worldToView * fh_normal;
        result = vec4(normalize(transformed_normal.xyz) * 0.5 + 0.5, 1.0);
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
        result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
        result.a = result.a + (1.0 - result.a) * color.a;
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