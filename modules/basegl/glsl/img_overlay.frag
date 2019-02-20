/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifdef ADDITIONAL_COLOR_LAYERS
ADDITIONAL_COLOR_LAYER_OUT_UNIFORMS
#endif

uniform sampler2D color_;
uniform sampler2D depth_;
uniform sampler2D picking_;

uniform int borderWidth_;
uniform vec4 borderColor_;

uniform vec4 viewport_ = vec4(0, 128, 128, 128);

in vec3 texCoord_;

void main() {
    // if border is present, we need to adjust the texture coords
    vec2 texCoord = gl_FragCoord.xy - viewport_.xy;

    // The BorderValue is used as a step function for determining the output results by mix.
    // If set to false, the textures will be sampled, otherwise the border color is set.
    bool border = false; 
    if (borderWidth_ > 0) {
        // set border flag iff the fragment coord is within the border
        vec2 centeredPos = (texCoord - viewport_.zw * 0.5);
        vec2 outputDim = viewport_.zw - vec2(2 * borderWidth_);

        if (any(greaterThan(abs(centeredPos), outputDim * 0.5))) {
            border = true;
        }
        // adjust texture coords
        // rescaling to size without border around the center
        texCoord += (outputDim - viewport_.zw) * 0.5;
    }

    // normalize texture coords with respect to size of the color texture
    // we assume that picking and depth texture have the same resolution
    texCoord /=  vec2(textureSize(color_, 0));

#ifdef SINGLE_CHANNEL
    vec4 srcColor = vec4(vec3(texture(color_, texCoord).r), 1.0);
#else
    vec4 srcColor = texture(color_, texCoord);
#endif

#if !defined(BLENDMODE_REPLACE)
    // if a blend mode other than "replace" is used, discard output for transparent fragments
    if ((srcColor.a == 0.0) && !border) {
        discard;
    }
#endif

    FragData0 = mix(srcColor, borderColor_, bvec4(border));

#ifdef ADDITIONAL_COLOR_LAYERS
    ADDITIONAL_COLOR_LAYER_WRITE
#endif

    PickingData = mix(texture(picking_, texCoord), vec4(0.0), bvec4(border));
    gl_FragDepth = mix(texture(depth_, texCoord).r, 1.0, border);
}
