#ifndef GLSL_VERSION_150
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_geometry_shader4 : enable
#endif

#include "pcp_common.glsl"

layout(lines) in;
layout(triangle_strip, max_vertices = 24) out;

in vec3 pickColor[2];
in vec2 texCoord[2];
vec4 triverts[4];
float signValues[4];

out vec3 lpickColor;
out vec2 ltexCoord;
out float lfalloffAlpha;

uniform float lineWidth;

uniform int selected;
uniform float selectedLineWidth = 3;

void emitV(int i) {
    gl_Position = triverts[i];
    lfalloffAlpha = signValues[i];
    lpickColor = pickColor[i % 2].rgb;
    ltexCoord = texCoord[i % 2].xy;
    EmitVertex();
}

void emit(int a, int b, int c, int d) {
    emitV(a);
    emitV(b);
    emitV(c);
    emitV(d);
    EndPrimitive();
}

void main() {
    // Compute orientation vectors for the two connecting faces:
    vec4 p[2];

#ifndef GLSL_VERSION_150
    p[0] = gl_PositionIn[0];
    p[1] = gl_PositionIn[1];
#else
    p[0] = gl_in[0].gl_Position;
    p[1] = gl_in[1].gl_Position;
#endif

    //* selected * selectedLineWidth
    // Assuming 2d
    vec3 j = vec3(0, 1, 0);
    float r = lineWidth * getPixelSpacing().y;
    if (selected == 1) {
        r = selectedLineWidth * getPixelSpacing().y;
    } else {
        r = lineWidth * getPixelSpacing().y;
    }

    j = vec3(0, r, 0);

    // Compute upper triangles
    signValues[0] = 1.0;
    triverts[0] = vec4(p[0].xyz, 1);
    signValues[1] = 1.0;
    triverts[1] = vec4(p[1].xyz, 1);
    signValues[2] = 0.0;
    triverts[2] = vec4(p[1].xyz + j * 1.0f, 1);
    signValues[3] = 0.0;
    triverts[3] = vec4(p[0].xyz + j * 1.0f, 1);
    emit(0, 1, 3, 2);

    // Compute lower triangles
    signValues[0] = 1.0;
    triverts[0] = vec4(p[0].xyz, 1);
    signValues[1] = 1.0;
    triverts[1] = vec4(p[1].xyz, 1);
    signValues[2] = 0.0;
    triverts[2] = vec4(p[1].xyz + j * -1.0f, 1);
    signValues[3] = 0.0;
    triverts[3] = vec4(p[0].xyz + j * -1.0f, 1);
    emit(0, 1, 3, 2);
}
