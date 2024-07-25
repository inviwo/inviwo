/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#define PI 3.141592653
#define TWOPI 6.283185307

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
void project(layout(r32i) iimage2DArray coeffTex, int N, float depth, float val)
#else
void project(layout(size1x32) image2DArray coeffTex, int N, float depth, float val)
#endif
{
    float costheta = cos(TWOPI * depth);
    float sintheta = sin(TWOPI * depth);
    float coskm1theta = 1.0;
    float sinkm1theta = 0.0;
    float cosktheta = 0.0;
    float sinktheta = 0.0;

    for (int i = 0; i < N; i++) {
        float projVal = 0.0;
        if (i == 0) {
            projVal = val;
        } else if (i % 2 == 0) {
            cosktheta = coskm1theta * costheta - sinkm1theta * sintheta;
            projVal = val * cosktheta;
        } else {
            sinktheta = sinkm1theta * costheta + coskm1theta * sintheta;
            projVal = val * sinktheta;
        }

        if (i % 2 == 0) {
            if (i != 0) {
                coskm1theta = cosktheta;
                sinkm1theta = sinktheta;
            }
        }

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        #ifdef COEFF_TEX_FIXED_POINT_FACTOR
            imageAtomicAdd(coeffTex, coord, int(projVal * COEFF_TEX_FIXED_POINT_FACTOR));
        #else
            float currVal = imageLoad(coeffTex, coord).x;
            imageStore(coeffTex, coord, vec4(currVal + projVal));
        #endif
    }
}

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
float approximate(layout(r32i) iimage2DArray coeffTex, int N, float depth)
#else
float approximate(layout(size1x32) image2DArray coeffTex, int N, float depth)
#endif
{
    float sum = 0.0;
    int k = 0;
    for (int i = 0; i < N; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        #ifdef COEFF_TEX_FIXED_POINT_FACTOR
            float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
        #else
            float coeff = imageLoad(coeffTex, coord).x;
        #endif
        if (i == 0) {
            sum += coeff * depth;
        } else if (i % 2 == 0) {
            sum += (coeff / (PI * k)) * sin(TWOPI * k * depth);
        } else {
            sum += (coeff / (PI * k)) * (1 - cos(TWOPI * k * depth));
        }
        if (i % 2 == 0) k++;
    }

    return sum;
}

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
float total(layout(r32i) iimage2DArray coeffTex, int N)
#else
float total(layout(size1x32) image2DArray coeffTex, int N)
#endif
{
    #ifdef COEFF_TEX_FIXED_POINT_FACTOR
        return float(imageLoad(coeffTex, ivec3(gl_FragCoord.xy, 0)).x) / COEFF_TEX_FIXED_POINT_FACTOR;
    #else
        return imageLoad(coeffTex, ivec3(gl_FragCoord.xy, 0)).x;
    #endif
}
