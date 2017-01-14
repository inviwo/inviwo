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

#if !defined(ENABLE_ADJACENCY)
#  define ENABLE_ADJACENCY 1
#endif

#if ENABLE_ADJACENCY == 1
layout(lines_adjacency) in;
#else
layout(lines) in;
#endif

layout(triangle_strip, max_vertices=5) out;


uniform GeometryParameters geometry_;
uniform CameraParameters camera_;

uniform vec2 screenDim_ = vec2(512, 512);
uniform float antialias_ = 1.0; // width of antialised edged [pixel]
uniform float lineWidth_ = 2.0; // line width [pixel]
uniform float miterLimit_ = 0.8; // limit for miter joins, i.e. cutting off joints between parallel lines 


in vec4 vertexColor_[];
in vec4 worldPosition_[];

out float segmentLength_; // length of the current line segment in screen space
out float objectLength_;  // length of line segment in world space
out vec2 texCoord_;
out vec4 color_;

//
// 2D line rendering in screen space.
//
// Loosley based on the publication by Nicolas P. Rougier[1] and Cinder-Samples code 
// provided by paulhoux on github[2]
//
// [1] Nicolas P. Rougier, Shader-Based Antialiased, Dashed, Stroked Polylines, 
//     Journal of Computer Graphics Techniques (JCGT), vol. 2, no. 2, 105--121, 2013 
//     http://jcgt.org/published/0002/02/08/
//
// [2] https://github.com/paulhoux/Cinder-Samples/tree/master/GeometryShader
//


// project p onto the line p0-p1 and return the projected length to p0
float projectedDistance(vec2 p0, vec2 p1, vec2 p) {
    return dot(p - p0, p1 - p0) / length(p1 - p0);
}

// emit vertex data consisting of position in NDC, texture coord, and color
void emit(in vec4 pos, in vec2 texCoord, in vec4 color, in float ndcToWorldFactor) {
    gl_Position = pos;
    texCoord_ = texCoord;
    color_ = color;
    objectLength_ = texCoord.x * ndcToWorldFactor;
    EmitVertex();
}

// render the current line segment without joints
void renderLineWithoutJoints() {
#if ENABLE_ADJACENCY == 1
#  define INDEX1 1
#  define INDEX2 2
#else
#  define INDEX1 0
#  define INDEX2 1
#endif

    vec4 pStart = gl_in[INDEX1].gl_Position / gl_in[INDEX1].gl_Position.w;
    vec4 pEnd = gl_in[INDEX2].gl_Position / gl_in[INDEX2].gl_Position.w;

    // line direction in screen space (2D)
    vec2 v = normalize(pEnd.xy - pStart.xy);

    // determine normal
    vec2 normal = vec2(-v.y, v.x);

    float halfWidth = lineWidth_ * 0.5;

    vec4 offset = vec4(normal * halfWidth, 0.0, 0.0);
    segmentLength_ = length(pEnd - pStart);
    // segment length in world space
    float lineLength = length(worldPosition_[INDEX2] - worldPosition_[INDEX1]);
    // scaling factor to convert line lengths in normalized device coords back to model space
    // this is used for reparametrization of the line
    float ndcToWorldFactor = lineLength / segmentLength_;

    emit(pStart + offset, vec2(0.0, halfWidth), vertexColor_[INDEX1], ndcToWorldFactor);
    emit(pStart - offset, vec2(0.0, -halfWidth), vertexColor_[INDEX1], ndcToWorldFactor);

    emit(pEnd + offset, vec2(segmentLength_, halfWidth), vertexColor_[INDEX2], ndcToWorldFactor);
    emit(pEnd - offset, vec2(segmentLength_, -halfWidth), vertexColor_[INDEX2], ndcToWorldFactor);

    EndPrimitive();
}

void main(void) {

#if ENABLE_ADJACENCY == 0
    // regular line rendering
    renderLineWithoutJoints();
#else
    vec2 screenDim = screenDim_ * 0.5;

    // Get the four vertices passed to the shader
    vec2 p0 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
    vec2 p1 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
    vec2 p2 = gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;
    vec2 p3 = gl_in[3].gl_Position.xy / gl_in[3].gl_Position.w;
    float linewidth = lineWidth_ / screenDim.x;
    float w = linewidth / 2.0 + 1.5*antialias_ / screenDim.x;

    segmentLength_ = length(p2-p1);

    // determine line directions
    vec2 v0 = normalize(p1 - p0); // previous segment
    vec2 v1 = normalize(p2 - p1); // current segment
    vec2 v2 = normalize(p3 - p2); // next segment
    // compute normals for the three segments
    vec2 n0 = vec2(-v0.y, v0.x);
    vec2 n1 = vec2(-v1.y, v1.x);
    vec2 n2 = vec2(-v2.y, v2.x);

    // angle between previous and current segment
    float d0 = sign(dot(v0, v1));
    // angle between current and next segment
    float d1 = sign(dot(v1, v2));

    // Determine miter lines by averaging the normals of the 2 segments
    vec2 miterBegin = normalize(n0 + n1); // miter at start of current segment
    vec2 miterEnd = normalize(n1 + n2); // miter at end of current segment

    // segment length in world space
    float lineLength = length(worldPosition_[2] - worldPosition_[1]);
    // scaling factor to convert line lengths in normalized device coords back to model space
    // this is used for reparametrization of the line
    float ndcToWorldFactor = lineLength / segmentLength_;

    // Determine the length of the miter by projecting it onto normal
    float length_a = w / dot(miterBegin, n1);
    float length_b = w / dot(miterEnd, n1);

    bool capBegin = (p0 == p1);
    bool capEnd = (p2 == p3);

    vec2 depth = vec2(gl_in[1].gl_Position.z / gl_in[1].gl_Position.w,
                      gl_in[2].gl_Position.z / gl_in[2].gl_Position.w);
    float slopeDepth = (depth.y - depth.x) / segmentLength_;

    // avoid sharp corners by cutting them off
    // corner between previous segment and current one
    if( dot( v0, v1 ) < -miterLimit_ ) {
        miterBegin = normalize(-n0 + n1);
        length_a = linewidth * 0.5;

        length_a = w / dot(miterBegin, n1);
    }
    // corner between current segment and next one
    if( dot( v1, v2 ) < -miterLimit_ ) {
        miterEnd = normalize(-n2 + n1);
        length_b = linewidth * 0.5;

        length_b = w / dot(miterEnd, n1);
    }

    vec2 leftTop, leftBottom;
    vec2 texCoord;
    if (capBegin) {
        // offset start position p1 by radius
        leftTop = p1 - w * v1 + w * n1;
        leftBottom = p1 - w * v1 - w * n1;
        texCoord = vec2(-w);
    }
    else {
        leftTop = p1 + length_a * miterBegin;
        leftBottom = p1 - length_a * miterBegin;
        texCoord.x = projectedDistance(p1, p2, leftTop);
        texCoord.y = projectedDistance(p1, p2, leftBottom);
    }

    vec2 vertexDepth = slopeDepth * texCoord + depth.x;

    objectLength_ = texCoord.x/segmentLength_ * lineLength;
    emit(vec4(leftTop, vertexDepth.x, 1.0), vec2(texCoord.x, w), vertexColor_[1], ndcToWorldFactor);
    
    objectLength_ = texCoord.y/segmentLength_ * lineLength;
    emit(vec4(leftBottom, vertexDepth.y, 1.0), vec2(texCoord.y, -w), vertexColor_[1], ndcToWorldFactor);


    vec2 rightTop, rightBottom;
    if (capEnd) {
        // offset end position p2 by radius
        rightTop = p2 + w * v1 + w * n1;
        rightBottom = p2 + w * v1 - w * n1;
        texCoord = vec2(segmentLength_ + w);
    }
    else {
        rightTop = p2 + length_b * miterEnd;
        rightBottom = p2 - length_b * miterEnd;
        texCoord.x = projectedDistance(p1, p2, rightTop);
        texCoord.y = projectedDistance(p1, p2, rightBottom);
    }

    vertexDepth = slopeDepth * texCoord + depth.x;

    objectLength_ = texCoord.x/segmentLength_ * lineLength;
    emit(vec4(rightTop, vertexDepth.x, 1.0), vec2(texCoord.x, w), vertexColor_[2], ndcToWorldFactor);

    objectLength_ = texCoord.y/segmentLength_ * lineLength;
    emit(vec4(rightBottom, vertexDepth.y, 1.0), vec2(texCoord.y, -w), vertexColor_[2], ndcToWorldFactor);

    EndPrimitive();
#endif
}
