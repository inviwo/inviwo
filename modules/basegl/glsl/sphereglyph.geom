/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

// Owned by the SphereRenderer Processor

#include "utils/structs.glsl"
#include "utils/pickingutils.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;

// define HEXAGON for hexagonal glyphs instead of image-space quads
//#define HEXAGON

layout(points) in;
#ifdef HEXAGON
    layout(triangle_strip, max_vertices = 6) out;
#else
    layout(triangle_strip, max_vertices = 4) out;
#endif

in vec4 worldPosition_[];
in vec4 sphereColor_[];
flat in float sphereRadius_[];
flat in uint pickID_[];

out float radius_;
out vec3 camPos_;
out vec4 center_;
out vec4 color_;
flat out vec4 pickColor_;

void main(void) {
    center_ = worldPosition_[0];
    radius_ = sphereRadius_[0];

    if (radius_ <= 0 || sphereColor_[0].a <= 0) {
        EndPrimitive();
        return;
    }

    mat4 worldToViewMatrixInv = inverse(camera.worldToView);

    vec3 camDir = normalize((worldToViewMatrixInv[2]).xyz);     
    vec3 camPosModel =  worldToViewMatrixInv[3].xyz;
    // calculate cam position (in model space of the sphere)
    camPos_ = camPosModel - center_.xyz;

#ifdef DISCARD_CLIPPED_GLYPHS
    if (dot(camPos_, camDir) < camera.nearPlane + radius_) {
        // glyph intersects with the near plane of the camera, discard entire glyph, i.e. no output
        EndPrimitive();
        return;
    }
#endif // DISCARD_CLIPPED_GLYPHS

    vec4 centerMVP = camera.worldToClip * center_;
    float glyphDepth = centerMVP.z / centerMVP.w;

    // send color to fragment shader
    color_ = sphereColor_[0];
    // set picking color
    pickColor_ = vec4(pickingIndexToColor(pickID_[0]), pickID_[0] == 0 ? 0.0 : 1.0);

    // camera coordinate system in object space
    vec3 camUp = (worldToViewMatrixInv[1]).xyz;
    vec3 camRight = normalize(cross(camDir, camUp));
    camUp = normalize(cross(camDir, camRight));


    float rad2 = radius_*radius_;
    float depth = 0.0;

    vec4 projPos;
    vec3 testPos;
    vec2 d, d_div;
    vec2 h, p, q;
    vec3 c2;
    vec3 cpj1, cpm1;
    vec3 cpj2, cpm2;

#ifdef HEXAGON

    float r_hex = 1.15470*radius_; // == 2.0/3.0 * sqrt(3.0)
    float h_hex = r_hex * 0.86602540; // == 1/2 * sqrt(3.0)

    camRight *= r_hex;
    vec3 camRight_half = camRight * 0.5;
    camUp *= h_hex;

    testPos = center_.xyz - camRight;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz - camRight_half - camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz - camRight_half + camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz + camRight_half - camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz + camRight_half + camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz + camRight;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

#else // HEXAGON
    // square:
    camRight *= radius_ * 1.41421356;
    camUp *= radius_ * 1.41421356;

    testPos = center_.xyz + camRight - camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz - camRight - camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz + camRight + camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();

    testPos = center_.xyz - camRight + camUp;
    projPos = camera.worldToClip * vec4(testPos, 1.0);
    projPos /= projPos.w;
    gl_Position = vec4(projPos.xy, glyphDepth, 1.0);
    EmitVertex();
#endif // HEXAGON

    EndPrimitive();
}
