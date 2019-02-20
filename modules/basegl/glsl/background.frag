/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

uniform sampler2D inportColor;

#ifdef ADDITIONAL_COLOR_LAYERS
ADDITIONAL_COLOR_LAYER_OUT_UNIFORMS
#endif // ADDITIONAL_COLOR_LAYERS
#ifdef PICKING_LAYER
uniform sampler2D inportPicking;
#endif // PICKING_LAYER
#ifdef DEPTH_LAYER
uniform sampler2D inportDepth;
#endif // PICKING_LAYER


uniform ImageParameters outportParameters;
uniform ivec2 checkerBoardSize = ivec2(10, 10);
uniform vec4 bgColor1 = vec4(0, 0, 0, 1);
uniform vec4 bgColor2 = vec4(1, 1, 1, 1);


#if !defined(SRC_COLOR) 
#  define SRC_COLOR vec4(0);
#endif // SRC_COLOR

#if !defined(BACKGROUND_STYLE_FUNCTION)
#  define BACKGROUND_STYLE_FUNCTION linearGradientVertical(texCoord)
#endif // BACKGROUND_STYLE_FUNCTION

#if !defined(BLENDFUNC)
#  define BLENDFUNC blendBackToFront
#endif // BLENDFUNC

vec4 checkerBoard(vec2 texCoord) {
    vec2 t = floor(ivec2(gl_FragCoord.x, outportParameters.dimensions.y - gl_FragCoord.y) /
                   checkerBoardSize);
    return mix(bgColor2, bgColor1, mod(t.x + t.y, 2.0) < 1.0 ? 1.0 : 0.0);
}

vec4 linearGradientHorizontal(vec2 texCoord) {
    return mix(bgColor2, bgColor1, texCoord.x);
}

vec4 linearGradientVertical(vec2 texCoord) {
    return mix(bgColor2, bgColor1, texCoord.y);
}

vec4 linearGradientSpherical(vec2 texCoord) {
	// bgColor1: inner color in circle
	// bgColor2: outer color
    return mix(bgColor1, bgColor2, distance(vec2(0.5), texCoord) / sqrt(0.5));
}


vec4 blendBackToFront(vec4 srcColor, vec4 dstColor) {
    return srcColor + dstColor * (1.0 - srcColor.a);
}

vec4 blendAlphaCompositing(vec4 srcColor, vec4 dstColor) {
    return mix(dstColor, srcColor, srcColor.a);
}


void main() {  
    vec2 texCoord = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec4 srcColor = SRC_COLOR;
    vec4 bgColor = BACKGROUND_STYLE_FUNCTION;

    // pre-multiplied alpha for background color
    bgColor.rgb *= bgColor.a;

    FragData0 = BLENDFUNC(srcColor, bgColor);


#ifdef ADDITIONAL_COLOR_LAYERS
    ADDITIONAL_COLOR_LAYER_WRITE
#endif

#ifdef PICKING_LAYER
    PickingData = texture(inportPicking, texCoord.xy);
#else
    PickingData = vec4(0);
#endif // PICKING_LAYER

#ifdef DEPTH_LAYER
    gl_FragDepth = texture(inportDepth, texCoord.xy).r;
#else
    // no depth input, reset depth to largest value
    gl_FragDepth = 1.0;
#endif // DEPTH_LAYER
}
