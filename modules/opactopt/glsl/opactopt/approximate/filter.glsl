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

 uniform int radius;
 layout(std430, binding = 8) buffer gaussianKernelBuffer {
    float kernel[];
};

void filterImportanceSum() {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    if (getPixelLink(coords) == 0) return;

    memoryBarrierImage();
    for (int i = 0; i < N_APPROXIMATION_COEFFICIENTS; i++) {
        ivec3 layer_coord = ivec3(coords, i);

        float val = 0.0;
        float kernel_sum = 0.0;
        for (int j = -radius; j <= radius; j++) {
            for (int k = -radius; k <= radius; k++) {
                if (coords.x + j < 0 || coords.x + j >= AbufferParams.screenWidth ) continue;
                if (coords.y + k < 0 || coords.y + k >= AbufferParams.screenHeight) continue;
                val += kernel[radius + j] * kernel[radius + k] * imageLoad(importanceSumCoeffs[0], layer_coord + ivec3(j, k, 0)).x;
                kernel_sum += kernel[radius + j] * kernel[radius + k];
            }
        }
        val /= kernel_sum;
        memoryBarrierImage();
        imageStore(importanceSumCoeffs[1], layer_coord, vec4(val));
        memoryBarrierImage();
    }
    memoryBarrierImage();
}
