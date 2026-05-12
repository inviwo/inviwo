/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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
//! #version 460
#include "utils/structs.glsl" //! #include "../../opengl/glsl/utils/structs.glsl"
#include "utils/depth.glsl"   //! #include "../../opengl/glsl/utils/depth.glsl"

#ifdef USE_FRAGMENT_LIST
#include "oit/abufferlinkedlist.glsl" 

// this is important for the occlusion query
layout(early_fragment_tests) in;

layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

#if !defined M_PI
#define M_PI 3.14159265358979323846
#endif

uniform vec2 screenDim = vec2(512, 512);
uniform float antialiasing = 0.5;  // width of antialised edged [pixel]

// initialize camera matrices with the identity matrix to enable default rendering
// without any transformation, i.e. all lines in clip space
uniform CameraParameters camera =
    CameraParameters(mat4(1), mat4(1), mat4(1), mat4(1), mat4(1), mat4(1), vec3(0), 0, 1);

// line stippling
uniform StipplingParameters stippling = StipplingParameters(30.0, 10.0, 0.0, 4.0);

in LineGeom {
    vec2 texCoord; // x = distance to segment start, y = orth. distance to center (in screen coords)
    vec4 color;
    flat vec4 pickColor;
    float segmentLength; // total length of the current line segment in screen space
    float distanceWorld;  // distance in world coords to segment start
    float lineWidthHalf;  // interpolated half line width for antialiasing
} fragment;

void main() {
    vec4 color = fragment.color;
    if (color.a < 0.01) discard;
    
    float lineWidthHalf = fragment.lineWidthHalf;

    // make joins round by using the texture coords
    float distance = abs(fragment.texCoord.y);
    if (fragment.texCoord.x < 0.0) {
        distance = length(fragment.texCoord);
    } else if (fragment.texCoord.x > fragment.segmentLength) {
        distance = length(vec2(fragment.texCoord.x - fragment.segmentLength, fragment.texCoord.y));
    }

    float d = distance - lineWidthHalf + antialiasing;

    // apply pseudo lighting
#if defined(ENABLE_PSEUDO_LIGHTING)
    float scaling = 0.8;
    color.rgb *= cos(distance / (lineWidthHalf + antialiasing * 1.2) * M_PI * 0.5 * scaling);
#endif

    float alpha = 1.0;

    // line stippling
#if defined(ENABLE_STIPPLING)
    float stippleLength = stippling.stippleLength + stippling.spacing;
#if STIPPLE_MODE == 2
    // in world space
    float v = (fragment.distanceWorld * stippling.worldScale);
#else
    // in screen space
    float v = (fragment.texCoord.x + stippling.offset) / stippleLength;
#endif  // STIPPLE_MODE

    float t = fract(v) * stippleLength / stippling.spacing;
    if ((t > 0.0) && (t < 1.0)) {
        // renormalize t with respect to stippling length
        t = min(t, 1.0 - t) * stippling.spacing * 0.5;
        d = max(d, t);
    }
#endif  // ENABLE_STIPPLING

    // antialiasing around the edges
    if (d > 0) {
        // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics Techniques 2013]
        d /= antialiasing;
        alpha = clamp(exp(-d * d), 0, 1);
    }

    color.a *= alpha;

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    // correct depth for a round profile, i.e. tube like appearance
    float depth = convertDepthScreenToView(camera, gl_FragCoord.z);

    float maxDist = (lineWidthHalf + antialiasing * 1.2);
    // assume circular profile of line
    depth = convertDepthViewToScreen(camera, 
        depth - cos(distance / maxDist * M_PI * 0.5) * maxDist / min(screenDim.x, screenDim.y));
#else
    float depth = gl_FragCoord.z;
#endif // ENABLE_ROUND_DEPTH_PROFILE

#if defined(USE_FRAGMENT_LIST)
    // fragment list rendering
    if (color.a > 0.0) {
        ivec2 coords = ivec2(gl_FragCoord.xy);
        abufferMeshRender(coords, depth, color);
    }
    discard;

#else  // USE_FRAGMENT_LIST
#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    gl_FragDepth = depth;
#endif  // ENABLE_ROUND_DEPTH_PROFILE
    FragData0 = color;
    PickingData = fragment.pickColor;
#endif // not USE_FRAGMENT_LIST
}
