/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

// enable conservative depth writes (supported since GLSL 4.20)
#if defined(GLSL_VERSION_450) || defined(GLSL_VERSION_440) || defined(GLSL_VERSION_430) || defined(GLSL_VERSION_420)
layout (depth_less) out float gl_FragDepth;
#endif

uniform vec2 screenDim = vec2(512, 512);
uniform float antialising = 1.0; // width of antialised edged [pixel]
uniform float lineWidth = 2.0; // line width [pixel]
uniform CameraParameters camera;

in float segmentLength_; // length of the current line segment in screen space
in float objectLength_;  // length of line segment in world space
in vec2 texCoord_;
in vec4 color_;

float reconstructDepth(float z) {
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return Zn*Zf / (Zf - z*(Zf - Zn));
}

float computeDepth(float z) {
    // compute depth in [-1,1]
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;
    float depth = (Zf + Zn) / (Zf - Zn) + (-2.0 * Zf * Zn) / (z * (Zf - Zn));
    return (depth + 1.0) * 0.5;
    return depth;
}

void main() {
    vec4 color = color_;

    float linewidthHalf = lineWidth * 0.5;

    // make joins round by using the texture coords
    float distance = abs(texCoord_.y);
    if (texCoord_.x < 0.0) { 
        distance = length(texCoord_); 
    }
    else if(texCoord_.x > segmentLength_) { 
        distance = length(vec2(texCoord_.x - segmentLength_, texCoord_.y)); 
    }

    float d = distance * screenDim.x * 0.5 - linewidthHalf + antialising;

    // apply pseudo lighting
    color.rgb *= cos(distance * screenDim.x * 0.5 / (linewidthHalf + antialising) * 1.2);

    float alpha = 1.0;
    // line stippling
    // if (int(objectLength_ * 5.0) % 4 == 0) {
    //    alpha = 0.0; 
    //    d = -0.2;      
    // }
    if( d > 0) {
        // apply antialising by modifying the alpha [Rougier, Journal of Computer Graphics Techniques 2013]
        d /= antialising;
        alpha = exp(-d*d);
    }
    // prevent fragments with low alpha from being rendered
    if (alpha < 0.2) discard;

    color.a = alpha;
    FragData0 = color;

    // correct depth
    float depth = reconstructDepth(gl_FragCoord.z);
    float maxDist = (linewidthHalf + antialising) / screenDim.x * 2.0;
    // assume circular profile of line
    gl_FragDepth = computeDepth(depth - cos(distance/maxDist) * maxDist);    
}
