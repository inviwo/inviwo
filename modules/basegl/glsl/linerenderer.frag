/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include "utils/depth.glsl"

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
// enable conservative depth writes (supported since GLSL 4.20)
#   if defined(GLSL_VERSION_450) || defined(GLSL_VERSION_440) || defined(GLSL_VERSION_430) || defined(GLSL_VERSION_420)
        layout (depth_less) out float gl_FragDepth;
#   endif

#if !defined M_PI
#define M_PI 3.14159265358979323846
#endif
#endif

uniform vec2 screenDim = vec2(512, 512);
uniform float antialiasing = 0.5; // width of antialised edged [pixel]
uniform float lineWidth = 2.0; // line width [pixel]

// initialize camera matrices with the identity matrix to enable default rendering
// without any transformation, i.e. all lines in clip space
uniform CameraParameters camera = CameraParameters( mat4(1), mat4(1), mat4(1), mat4(1),
                                    mat4(1), mat4(1), vec3(0), 0, 1);


// line stippling
uniform StipplingParameters stippling = StipplingParameters(30.0, 10.0, 0.0, 4.0);

in LineGeom {
    vec2 texCoord; // x = distance to segment start, y = orth. distance to center (in screen coords)
    vec4 color;
    flat vec4 pickColor;
    float segmentLength; // total length of the current line segment in screen space
    float distanceWorld;  // distance in world coords to segment start
} fragment;

void main() {
    vec4 color = fragment.color;

    float linewidthHalf = lineWidth * 0.5;

    // make joins round by using the texture coords
    float distance = abs(fragment.texCoord.y);
    if (fragment.texCoord.x < 0.0) { 
        distance = length(fragment.texCoord); 
    }
    else if(fragment.texCoord.x > fragment.segmentLength) { 
        distance = length(vec2(fragment.texCoord.x - fragment.segmentLength, fragment.texCoord.y)); 
    }

    float d = distance - linewidthHalf + antialiasing;

    // apply pseudo lighting
#if defined(ENABLE_PSEUDO_LIGHTING)
    color.rgb *= cos(distance / (linewidthHalf + antialiasing) * 1.2);
#endif

    float alpha = 1.0;

    // line stippling
#if defined(ENABLE_STIPPLING)
    float stippleLength = stippling.length + stippling.spacing;
#if STIPPLE_MODE == 2
    // in world space
    float v = (fragment.distanceWorld * stippling.worldScale);
#else
    // in screen space
    float v = (fragment.texCoord.x + stippling.offset) / stippleLength;
#endif // STIPPLE_MODE

    float t = fract(v) * stippleLength / stippling.spacing;
    if ((t > 0.0) && (t < 1.0)) {
        // renormalize t with respect to stippling length
        t = min(t, 1.0 - t) * stippling.spacing * 0.5;
        d = max(d, t);
    }
#endif // ENABLE_STIPPLING

    // antialiasing around the edges
    if( d > 0) {
        // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics Techniques 2013]
        d /= antialiasing;
        alpha = exp(-d*d);
    }
    // prevent fragments with low alpha from being rendered
    if (alpha < 0.02) discard;

    color.rgb *= color.a;
    FragData0 = color * alpha;

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    // correct depth for a round profile, i.e. tube like appearance
    float depth = convertDepthScreenToView(camera, gl_FragCoord.z);
    float maxDist = (linewidthHalf + antialiasing);
    // assume circular profile of line
    gl_FragDepth = convertDepthViewToScreen(camera, 
        depth - cos(distance / maxDist * M_PI) * maxDist / screenDim.x * 0.5);
#endif // ENABLE_ROUND_DEPTH_PROFILE

    PickingData = fragment.pickColor;
}
