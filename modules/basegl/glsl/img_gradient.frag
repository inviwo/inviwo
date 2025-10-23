/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include "utils/gradients.glsl"

uniform sampler2D inport;
uniform ImageParameters inportParameters;
uniform ImageParameters outportParameters;

uniform mat3 inverseMetricTensor = mat3(1.0);
uniform int channel = 0;

vec2 gradientCentralDiffTest(sampler2D tex, ImageParameters texParams, in vec2 texcoord, 
                  in mat3 inverseMetricTensor, in int component) {
    vec2 offset = texParams.reciprocalDimensions * 0.5;
    vec2 d;
    d.x = getNormalizedTexel(tex, texParams, texcoord + vec2(offset.x, 0.0))[component]
          - getNormalizedTexel(tex, texParams, texcoord - vec2(offset.x, 0.0))[component];
    d.y = getNormalizedTexel(tex, texParams, texcoord + vec2(0.0, offset.y))[component]
          - getNormalizedTexel(tex, texParams, texcoord - vec2(0.0, offset.y))[component];
    d /= 2.0 * offset;

    // The gradient ∇f in world space is then computed based on the partial derivatives in u and v
    // direction
    //     ∇f = g_11 ∂f/∂u a_1 + g_12 ∂f/∂u a_2 + g_21 ∂f/∂v a_1 + g_22 ∂f/∂v a_2,
    // where g_ij refers to the inverse metric tensor and a_i to the ith basis vector.
    //
    // In the 2D case, we assume that the third basis vector is orthogonal to the first two. Thus, 
    // the g_31, g_32, g_13, and g_23 are zero and the out-of-plane partial derivative is zero as well.
      return mat2(inverseMetricTensor) * d;
}

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec2 gradient = gradientCentralDiffTest(inport, inportParameters, texCoords.xy, inverseMetricTensor, channel);
    FragData0 = vec4(gradient, 0, 1);
}
