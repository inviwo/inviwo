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
vec4 p[2];

out float axisXpos;

uniform int selected;

void emitV(int i) {
    gl_Position = triverts[i];
	axisXpos = p[0].x;
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

float axisWidth = 10;

	// Compute orientation vectors for the two connecting faces:
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
    float r = axisWidth * getPixelSpacing().x;

    j = vec3(r, 0, 0);

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
