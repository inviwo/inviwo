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

in vec4 color_;
in float radius_;
in vec3 camPos;
in vec4 center;

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
    coord -= center;
    // setup viewing ray
    vec3 ray = normalize(coord.xyz - camPos);
    
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

    // illumination
    vec4 glyphColor;
    glyphColor.rgb = APPLY_LIGHTING(light_, color_.rgb, color_.rgb, vec3(1.0f), intersection.xyz,
                               normalize(normal), normalize(camPos - intersection));
    glyphColor.a = color_.a;
    // color.rgb = APPLY_LIGHTING(light_, color.rgb, color.rgb, vec3(1.0f), worldPosition_.xyz,
    //                            normalize(normal), normalize(toCameraDir_));

    // depth correction for glyph
    mat4 mvpTranspose = transpose(camera_.worldToClip); // gl_ModelViewProjectionMatrixTranspose

    vec4 pos = vec4(intersection + center.xyz, 1.0);
    float depth = dot(mvpTranspose[2], pos);
    float depthW = dot(mvpTranspose[3], pos);

    depth = ((depth / depthW) + 1.0) * 0.5;

    if (depth <= 0.0) {
        // first intersection lies behind the camera, compute the second intersection
        // intersection = (d1 + sqrt(radicand))*ray + coord.xyz;
        // pos = vec4(intersection + center.xyz, 1.0);
        // depth = dot(mvpTranspose[2], pos);
        // depthW = dot(mvpTranspose[3], pos);

        // depth = max(((depth / depthW) + 1.0) * 0.5, 0.000001);
        depth = 0.00001;
        glyphColor.rgb = color_.rgb * 0.9;
    }

    FragData0 = glyphColor;
    gl_FragDepth = depth;
	PickingData = vec4(0);
}
