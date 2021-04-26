/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include "utils/pickingutils.glsl"

layout(points) in;

layout(triangle_strip, max_vertices=4) out;

uniform bool pickingEnabled = true; // disables color output
uniform vec4 color = vec4(0, 0, 0, 1);
uniform uint pickId;
uniform mat4 trafo;

uniform vec2 screenDim = vec2(512, 512);
uniform float antialiasing = 0.5; // width of antialised edged [pixel]
uniform float lineWidth = 2.0; // line width [pixel]
uniform bool roundCaps = true;

out vec2 texCoord_; // x = distance to segment start, y = orth. distance to center (in screen coords)
out float segmentLength_;
out vec4 color_;
flat out vec4 pickColor_;

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

// emit vertex data consisting of position in NDC, texture coord
void emit(in vec4 pos, in vec2 texCoord) {
    gl_Position = pos;
    texCoord_ = texCoord;
    EmitVertex();
}

vec2 convertNDCToScreen(vec2 v) {
    return (v + 1.0) * 0.5 * screenDim;
}

vec4 convertScreenToNDC(vec2 v, float z) {
    return vec4(v / screenDim * 2.0 - 1.0, z, 1.0);
}

void main() {
    vec2 halfScreenDim = screenDim * 0.5;

    vec4 startPos = trafo * vec4(0.0, 0.0, 0.0, 1.0);
    vec4 endPos = trafo * vec4(0.0, 1.0, 0.0, 1.0);

    // set pick color equivalent to first vertex
    color_ = (pickingEnabled ? vec4(0) : color);
    pickColor_ = vec4(pickingIndexToColor(pickId), pickId == 0 ? 0.0 : 1.0);
    
    vec4 p1ndc = startPos / startPos.w;
    vec4 p2ndc = endPos / endPos.w;

    vec2 p1 = convertNDCToScreen(p1ndc.xy);
    vec2 p2 = convertNDCToScreen(p2ndc.xy);

    // determine line direction and normal
    vec2 v1 = normalize(p2 - p1);
    vec2 n1 = vec2(-v1.y, v1.x);
    segmentLength_ = length(p2 - p1);

    float w = lineWidth * 0.5 + 1.2 * antialiasing;

    // compute start position at p1
    vec2 leftTop = p1 + w * n1;
    vec2 leftBottom = p1 - w * n1;
    vec2 texCoord = vec2(0);

    if (roundCaps) {
        // extend segment beyond p1 by radius for cap
        leftTop -= w * v1;
        leftBottom -= w * v1;
        texCoord -= w;
    }

    emit(convertScreenToNDC(leftTop, p1ndc.z), vec2(texCoord.x, w));
    emit(convertScreenToNDC(leftBottom, p1ndc.z), vec2(texCoord.y, -w));

    // compute end position at p2
    vec2 rightTop = p2 + w * n1;
    vec2 rightBottom = p2 - w * n1;
    texCoord = vec2(segmentLength_);

    if (roundCaps) {
        // extend segment beyond p2 by radius for cap
        rightTop += w * v1;
        rightBottom += w * v1;
        texCoord += w;
    }

    emit(convertScreenToNDC(rightTop, p2ndc.z), vec2(texCoord.x, w));
    emit(convertScreenToNDC(rightBottom, p2ndc.z), vec2(texCoord.y, -w));

    EndPrimitive();
}
