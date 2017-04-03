/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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
#include "utils/shading.glsl"

uniform GeometryParameters geometry_;
uniform CameraParameters camera_;
uniform LightParameters light_;

uniform vec4 viewport_; // holds viewport offset x, offset y, 2 / viewport width, 2 / viewport height
uniform float clipShadingFactor_ = 0.9;

in vec4 color_;
in float radius_;
in vec3 camPos_;
in vec4 center_;

#if !defined(APPLY_GLYPH_CLIPPING)
#  define APPLY_GLYPH_CLIPPING(coord, intersection, mvpTranspose, srcColor, dstColor, dstDepth) discard;
#endif

void clipToSolid(in vec4 coord, in vec3 srcColor, out vec4 dstColor, out float dstDepth) { 
    dstDepth = 0.000001;

#if defined(SHADE_CLIPPED_AREA)
    // need to compute proper ray-near plane intersection for shading.
    // Use initial glyph coordinate for now since this should be precise enough for smaller glyphs.

    // clip surface is orthogonal to view direction of the camera, use viewDir as normal
    vec3 normal = normalize((camera_.viewToWorld[2]).xyz);
    vec3 shadedColor = color_.rgb * clipShadingFactor_;
    dstColor.rgb = APPLY_LIGHTING(light_, srcColor, srcColor, vec3(1.0f), coord.xyz,
                           normal, normalize(camPos_ - coord.xyz));
#else
    dstColor.rgb = color_.rgb * clipShadingFactor_;
#endif // SHADE_CLIPPED_AREA
}

void clipToHollowGlyph(in vec3 intersection, in mat4 mvpTranspose, in vec3 srcColor, 
                       out vec4 dstColor, out float dstDepth) {
    vec4 pos = vec4(intersection + center_.xyz, 1.0);
    float depth = dot(mvpTranspose[2], pos);
    float depthW = dot(mvpTranspose[3], pos);
    dstDepth = max(((depth / depthW) + 1.0) * 0.5, 0.000001);

#if defined(SHADE_CLIPPED_AREA)
    vec3 normal = -intersection / radius_;
    dstColor.rgb = APPLY_LIGHTING(light_, srcColor, srcColor, vec3(1.0f), intersection.xyz,
                           normal, normalize(camPos_ - intersection));
#else
    dstColor.rgb = color_.rgb * clipShadingFactor_;
#endif // SHADE_CLIPPED_AREA
}

void main() {
    vec4 pixelPos = gl_FragCoord;
    pixelPos.xy -= viewport_.xy;
    // transform fragment coordinates from window coordinates to view coordinates.
    vec4 coord = pixelPos
        * vec4(viewport_.z, viewport_.w, 2.0, 0.0)
        + vec4(vec3(-1.0), 1.0);

    // transform fragment coord into object space
    //coord = gl_ModelViewProjectionMatrixInverse * coord;
    coord = camera_.clipToWorld * coord;
    coord /= coord.w;
    coord -= center_;
    // setup viewing ray
    vec3 ray = normalize(coord.xyz - camPos_);
    
    // calculate sphere-ray intersection
    // start ray at current coordinate and not at the camera
    float d1 = -dot(coord.xyz, ray);
    float d2s = dot(coord.xyz, coord.xyz) - d1*d1;
    float radicand = radius_*radius_ - d2s;
    
    if (radicand < 0.0) {
        // no valid intersection found
        discard;
    }

    // calculate intersection point
    vec3 intersection = (d1 - sqrt(radicand))*ray + coord.xyz;
    
    vec3 normal = intersection / radius_;

    // shading
    vec4 glyphColor;
    glyphColor.rgb = APPLY_LIGHTING(light_, color_.rgb, color_.rgb, vec3(1.0f), intersection,
                               normal, normalize(camPos_ - intersection));
    glyphColor.a = color_.a;

    // depth correction for glyph
    mat4 mvpTranspose = transpose(camera_.worldToClip);

    vec4 pos = vec4(intersection + center_.xyz, 1.0);
    float depth = dot(mvpTranspose[2], pos);
    float depthW = dot(mvpTranspose[3], pos);

    depth = ((depth / depthW) + 1.0) * 0.5;

    if (depth <= 0.0) {
        // first intersection lies behind the camera, compute the second intersection
        vec3 secondIntersection = (d1 + sqrt(radicand))*ray + coord.xyz;

        vec3 shadedColor = color_.rgb * clipShadingFactor_;
        APPLY_GLYPH_CLIPPING(coord, secondIntersection, mvpTranspose, shadedColor, glyphColor, depth);
    }

    FragData0 = glyphColor;
    gl_FragDepth = depth;
	PickingData = vec4(0);
}
