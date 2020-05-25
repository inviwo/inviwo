/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2020 Inviwo Foundation
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

#ifdef USE_FRAGMENT_LIST
#include "oit/abufferlinkedlist.glsl"

// this is important for the occlusion query
layout(early_fragment_tests) in;

layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
// enable conservative depth writes (supported since GLSL 4.20)
#if defined(GLSL_VERSION_450) || defined(GLSL_VERSION_440) || defined(GLSL_VERSION_430) || \
    defined(GLSL_VERSION_420)
layout(depth_less) out float gl_FragDepth;
#endif
#endif

#if defined(UNIFORM_ALPHA)
uniform float uniformAlpha;
#endif
#if defined(OVERWRITE_COLOR)
uniform vec4 overwriteColor;
#endif

uniform vec2 screenDim = vec2(512, 512);
uniform float antialiasing = 0.5;  // width of antialised edged [pixel]
uniform float lineWidth = 2.0;     // line width [pixel]

// initialize camera matrices with the identity matrix to enable default rendering
// without any transformation, i.e. all lines in clip space
uniform CameraParameters camera =
    CameraParameters(mat4(1), mat4(1), mat4(1), mat4(1), mat4(1), mat4(1), vec3(0), 0, 1);

// line stippling
uniform StipplingParameters stippling = StipplingParameters(30.0, 10.0, 0.0, 4.0);

in float segmentLength_;  // total length of the current line segment in screen space
in float distanceWorld_;  // distance in world coords to segment start
in vec2
    texCoord_;  // x = distance to segment start, y = orth. distance to center (in screen coords)
in vec4 color_;
flat in vec4 pickColor_;

void main() {
#if defined(OVERWRITE_COLOR)
    vec4 color = overwriteColor;

#else //UNIFORM_ALPHA
    vec4 color = color_;
#if defined(UNIFORM_ALPHA)
    color.a = uniformAlpha;
#endif // UNIFORM_ALPHA
#endif // not OVERWRITE_COLOR

    if (color.a < 0.01) discard;
    float linewidthHalf = lineWidth * 0.5;

    // make joins round by using the texture coords
    float distance = abs(texCoord_.y);
    if (texCoord_.x < 0.0) {
        distance = length(texCoord_);
    } else if (texCoord_.x > segmentLength_) {
        distance = length(vec2(texCoord_.x - segmentLength_, texCoord_.y));
    }

    float d = distance - linewidthHalf + antialiasing;

    // apply pseudo lighting
#if defined(ENABLE_PSEUDO_LIGHTING)
    color.rgb *= cos(distance / (linewidthHalf + antialiasing) * 1.2);
#endif

    float alpha = 1.0;

    // line stippling
#if defined(ENABLE_STIPPLING)

#if STIPPLE_MODE == 2
    // in world space
    float v = (distanceWorld_ * stippling.worldScale);
#else
    // in screen space
    float v = (texCoord_.x + stippling.offset) / stippling.length;
#endif  // STIPPLE_MODE

    float t = fract(v) * (stippling.length) / stippling.spacing;
    if ((t > 0.0) && (t < 1.0)) {
        // renormalize t with respect to stippling length
        t = min(t, 1.0 - t) * (stippling.spacing) * 0.5;
        d = max(d, t);
    }
#endif  // ENABLE_STIPPLING

    // antialiasing around the edges
    if (d > 0) {
        // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics
        // Techniques 2013]
        d /= antialiasing;
        alpha = exp(-d * d);
    }

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    // correct depth for a round profile, i.e. tube like appearance
    float depth = convertDepthScreenToView(camera, gl_FragCoord.z);
    float maxDist = (linewidthHalf + antialiasing);
    depth = depth - cos(distance / maxDist) * maxDist / screenDim.x * 0.5;
    depth = convertDepthViewToScreen(camera, depth);
#else
    float depth = gl_FragCoord.z;
#endif  // ENABLE_ROUND_DEPTH_PROFILE

#if defined(USE_FRAGMENT_LIST)
    // fragment list rendering
    if (color.a > 0.0) {
        ivec2 coords = ivec2(gl_FragCoord.xy);

        abufferRender(coords, depth, vec4(color.xyz, color.a *alpha));
    }
    discard;

#else  // USE_FRAGMENT_LIST

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    gl_FragDepth = depth;
#endif  // ENABLE_ROUND_DEPTH_PROFILE
    FragData0 = vec4(color.xyz, color.a * alpha);
    PickingData = pickColor_;
#endif // not USE_FRAGMENT_LIST
}
