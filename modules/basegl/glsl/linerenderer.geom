/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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
#include "utils/selectioncolor.glsl"

#if !defined(ENABLE_ADJACENCY)
#  define ENABLE_ADJACENCY 0
#endif

#if ENABLE_ADJACENCY == 1
layout(lines_adjacency) in;
#else
layout(lines) in;
#endif

layout(triangle_strip, max_vertices=5) out;

uniform vec2 screenDim = vec2(512, 512);
uniform float antialiasing = 0.5; // width of antialised edged [pixel]
uniform float lineWidth = 2.0; // line width [pixel]
uniform float miterLimit = 0.8; // limit for miter joins, i.e. cutting off joints between parallel lines 
uniform bool roundCaps = false;

#if defined(ENABLE_BNL)
uniform usamplerBuffer bnl;
uniform SelectionColor bnlFilter;
uniform SelectionColor bnlSelect;
uniform SelectionColor bnlHighlight;
#endif

in LineVert {
    vec4 worldPosition;
    vec4 color;
    flat uint pickID;
    flat uint index;
} inVertices[];


out LineGeom {
    vec2 texCoord; // x = distance to segment start, y = orth. distance to center (in screen coords)
    vec4 color;
    flat vec4 pickColor;
    float segmentLength; // total length of the current line segment in screen space
    float distanceWorld;  // distance in world coords to segment start
} outLine;

struct Vertex {
    vec4 pos;
    vec2 texCoord;
    vec4 color;
    float distanceWorld;
};

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

vec2 convertNDCToScreen(vec2 v) {
    return (v + 1.0) * 0.5 * screenDim;
}

vec4 convertScreenToNDC(vec2 v, float z) {
    return vec4(v / screenDim * 2.0 - 1.0, z, 1.0);
}

Vertex createVertex(in vec2 pos, in float depth, in vec2 texCoord, in vec4 color, in float screenToWorldFactor) {
    return Vertex(convertScreenToNDC(pos, depth),
        texCoord,
        color, 
        texCoord.x * screenToWorldFactor);
 }

// emit vertex data consisting of position in NDC, texture coord, and color
void emit(in Vertex vertex, in uint pickID, in float segmentLength) {
    gl_Position = vertex.pos;
    outLine.segmentLength = segmentLength;
    outLine.distanceWorld = vertex.distanceWorld;
    outLine.texCoord = vertex.texCoord;
    outLine.color = vertex.color;
    outLine.pickColor = vec4(pickingIndexToColor(pickID), pickID == 0 ? 0.0 : 1.0);
    EmitVertex();
}

// homogeneous clipping for line segments
// 
// @param p1, p2    describe line segment
// @param axis      determines plane which the line segment is clipped against (x = 0, y = 1, z = 2)
// @param sign      +1 for positive clip plane, -1 for negative clip plane
//   
// @result clipped line segmenet p1-p2
//
void homogeneousClip(inout vec4 p1, inout vec4 p2, int axis, float sign, 
                     inout bool p1Clipped, inout bool p2Clipped) {
    // clip against -y
    float t = (p1.w + sign * p1[axis]) / ((p1.w + sign * p1[axis]) - (p2.w + sign * p2[axis]));
    vec4 pNew = mix(p1, p2, t);

    if (sign*p1[axis] + p1.w > 0) {
        p2 = pNew;
        p2Clipped = true;
    } else {
        p1 = pNew;
        p1Clipped = true;
    }
}

#if defined(ENABLE_BNL)
void applyBrushingAndLinking(in uint index1, inout vec4 color1, 
                             in uint index2, inout vec4 color2) {
    int bnlSize = textureSize(bnl);
    uint flags1 = index1 < bnlSize ? texelFetch(bnl, int(index1)).x : uint(0);
    uint flags2 = index1 < bnlSize ? texelFetch(bnl, int(index2)).x : uint(0);

    if (flags1 == 3 || flags2 == 3) {
        color1 = applySelectionColor(color1, bnlFilter);
        color2 = applySelectionColor(color2, bnlFilter);
    } else if (flags1 == 2 || flags2 == 2) {
        color1 = applySelectionColor(color1, bnlHighlight);
        color2 = applySelectionColor(color2, bnlHighlight);
    } else if (flags1 == 1 && flags2 == 1) {
        color1 = applySelectionColor(color1, bnlSelect);
        color2 = applySelectionColor(color2, bnlSelect);
    }
}
#endif // ENABLE_BNL

void main(void) {
    vec2 halfScreenDim = screenDim * 0.5;

#if ENABLE_ADJACENCY == 0
    // no adjacency information available, duplicate first and second vertex
    const int index1 = 0;
    const int index2 = 1;

    vec4 p0in = gl_in[0].gl_Position;
    vec4 p1in = gl_in[0].gl_Position;
    vec4 p2in = gl_in[1].gl_Position;
    vec4 p3in = gl_in[1].gl_Position;
#else
    // Get the four vertices passed to the shader
    const int index1 = 1;
    const int index2 = 2;

    vec4 p0in = gl_in[0].gl_Position;
    vec4 p1in = gl_in[1].gl_Position;
    vec4 p2in = gl_in[2].gl_Position;
    vec4 p3in = gl_in[3].gl_Position;
#endif
    
    vec4 color1 = inVertices[index1].color;
    vec4 color2 = inVertices[index2].color;

#if defined(ENABLE_BNL)
    applyBrushingAndLinking(inVertices[index1].index, color1, inVertices[index2].index, color2);
#endif

    if (color1.a <= 0.0 && color2.a <= 0.0) {
        EndPrimitive();
        return;
    }


    // perform homogeneous clipping
    if (p1in.w * p2in.w < 0.0) {
        // TODO: ignore all segments intersecting with the near clip plane due 
        //       to bug in homogeneous clipping 
        return;
        /*
        bool p1Clipped = false;
        bool p2Clipped = false;
        
        // clip against w = 0
        float t = p1in.w / (p1in.w - p2in.w);
        vec4 pNew = mix(p1in, p2in, t);
        if (p1in.w > 0.0) {
            // replace p2 and p3
            p2in = pNew;
            p2Clipped = true;
        } else {
            // replace p1 and p0
            p1Clipped = true;
            p1in = pNew;
        }

        // clip against neg. y
        homogeneousClip(p1in, p2in, 1, -1, p1Clipped, p2Clipped);
        // clip against pos. y
        homogeneousClip(p1in, p2in, 1, +1, p1Clipped, p2Clipped);
        // clip against neg. x
        homogeneousClip(p1in, p2in, 0, -1, p1Clipped, p2Clipped);
        // clip against pos. x
        homogeneousClip(p1in, p2in, 0, +1, p1Clipped, p2Clipped);
        // clip against neg. z
        homogeneousClip(p1in, p2in, 2, -1, p1Clipped, p2Clipped);
        // clip against pos. z
        homogeneousClip(p1in, p2in, 2, +1, p1Clipped, p2Clipped);

        if (p1Clipped) {
            p0in = p1in;
        }
        if (p2Clipped) {
            p3in = p2in;
        }
        if (p1Clipped && p2Clipped);
        */
    }

    vec2 p0ndc = p0in.xy / p0in.w;
    vec4 p1ndc = p1in / p1in.w;
    vec4 p2ndc = p2in / p2in.w;
    vec2 p3ndc = p3in.xy / p3in.w;    

    vec2 p0 = convertNDCToScreen(p0ndc);
    vec2 p1 = convertNDCToScreen(p1ndc.xy);
    vec2 p2 = convertNDCToScreen(p2ndc.xy);
    vec2 p3 = convertNDCToScreen(p3ndc);

    // determine line directions
    vec2 v0 = normalize(p1 - p0); // previous segment
    vec2 v1 = normalize(p2 - p1); // current segment
    vec2 v2 = normalize(p3 - p2); // next segment
    // compute normals for the three segments
    vec2 n0 = vec2(-v0.y, v0.x);
    vec2 n1 = vec2(-v1.y, v1.x);
    vec2 n2 = vec2(-v2.y, v2.x);

    vec2 depth = vec2(p1ndc.z, p2ndc.z);

    float w = lineWidth * 0.5 + 1.2 * antialiasing;
    float segmentLength = length(p2 - p1);
    // segment length in world space
    float lineLengthWorld = length(inVertices[index2].worldPosition - inVertices[index1].worldPosition);
    // scaling factor to convert line lengths in screenspace coords back to model space
    // this is used for reparametrization of the line
    float screenToWorldFactor = lineLengthWorld / segmentLength;

    // angle between previous and current segment
    float d0 = sign(dot(v0, v1));
    // angle between current and next segment
    float d1 = sign(dot(v1, v2));

    // Determine miter lines by averaging the normals of the 2 segments
    vec2 miterBegin = normalize(n0 + n1); // miter at start of current segment
    vec2 miterEnd = normalize(n1 + n2); // miter at end of current segment

    // Determine the length of the miter by projecting it onto normal
    float length_a = w / dot(miterBegin, n1);
    float length_b = w / dot(miterEnd, n1);

    bool capBegin = (p0 == p1);
    bool capEnd = (p2 == p3);

    // depth delta is computed in NDC, but we need to apply the slope depth in screen space
    // i.e. normalization with respect to segmentLength and not length(p2ndc - p1ndc)
    float slopeDepth = (depth.y - depth.x) / segmentLength;

    // avoid sharp corners by cutting them off
    // corner between previous segment and current one
    if (dot(v0, v1) < -miterLimit) {
        miterBegin = normalize(-n0 + n1);
        length_a = lineWidth * 0.5;

        length_a = w / dot(miterBegin, n1);
    }
    // corner between current segment and next one
    if (dot(v1, v2) < -miterLimit) {
        miterEnd = normalize(-n2 + n1);
        length_b = lineWidth * 0.5;

        length_b = w / dot(miterEnd, n1);
    }

    vec2 leftTop, leftBottom;
    vec2 texCoord;
    if (capBegin) {
        // compute start position at p1
        leftTop = p1 + w * n1;
        leftBottom = p1 - w * n1;
        texCoord = vec2(0);

        if (roundCaps) {
            // extend segment beyond p1 by radius for cap
            leftTop -= w * v1;
            leftBottom -= w * v1;
            texCoord -= w;
        }
    }
    else {
        leftTop = p1 + length_a * miterBegin;
        leftBottom = p1 - length_a * miterBegin;
        texCoord.x = projectedDistance(p1, p2, leftTop);
        texCoord.y = projectedDistance(p1, p2, leftBottom);
    }

    vec2 vertexDepth = slopeDepth * texCoord + depth.x;
    // set pick ID equivalent to first vertex
    uint pickID = inVertices[index1].pickID;    

    Vertex vOut1 = createVertex(leftTop, vertexDepth.x, vec2(texCoord.x, w), 
                                color1, screenToWorldFactor);
    Vertex vOut2 = createVertex(leftBottom, vertexDepth.y, vec2(texCoord.y, -w), 
                                color1, screenToWorldFactor);

    emit(vOut1, pickID, segmentLength);
    emit(vOut2, pickID, segmentLength);

    vec2 rightTop, rightBottom;
    if (capEnd) {
        // compute end position at p2
        rightTop = p2 + w * n1;
        rightBottom = p2 - w * n1;
        texCoord = vec2(segmentLength);

        if (roundCaps) {
            // extend segment beyond p2 by radius for cap
            rightTop += w * v1;
            rightBottom += w * v1;
            texCoord += w;
        }
    }
    else {
        rightTop = p2 + length_b * miterEnd;
        rightBottom = p2 - length_b * miterEnd;
        texCoord.x = projectedDistance(p1, p2, rightTop);
        texCoord.y = projectedDistance(p1, p2, rightBottom);
    }

    vertexDepth = slopeDepth * texCoord + depth.x;

    Vertex vOut3 = createVertex(rightTop, vertexDepth.x, vec2(texCoord.x, w), 
                                color2, screenToWorldFactor);
    Vertex vOut4 = createVertex(rightBottom, vertexDepth.y, vec2(texCoord.y, -w), 
                                color2, screenToWorldFactor);

    emit(vOut3, pickID, segmentLength);
    emit(vOut4, pickID, segmentLength);

    EndPrimitive();
}
