
#include "pcp_common.glsl"
#include "utils/pickingutils.glsl"

layout(lines) in;
layout(triangle_strip, max_vertices = 24) out;

in float vScalarMeta[2];
flat in uint vPicking[2];

vec4 triverts[4];
float signValues[4];

out vec4 lPickColor;
out vec4 lMapedColor;
out float lFalloffAlpha;

uniform float lineWidth = 3;

uniform sampler2D tf;

void emitV(int i) {
    gl_Position = triverts[i];
    lFalloffAlpha = signValues[i];
    lPickColor = vec4(pickingIndexToColor(vPicking[i % 2]), vPicking[i % 2] == 0 ? 0.0 : 1.0);
    lMapedColor = texture(tf, vec2(vScalarMeta[i % 2], 0.5f));
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

    p[0] = gl_in[0].gl_Position;
    p[1] = gl_in[1].gl_Position;

    // Create a vector that is orthogonal to the line
    vec3 orthogonalLine = p[0].xyz - p[1].xyz;
    orthogonalLine = normalize(vec3(orthogonalLine.y, -orthogonalLine.x, orthogonalLine.z));

    // Scale the linewidth with the window dimensions
    float r1 = lineWidth * getPixelSpacing().x;
    float r2 = lineWidth * getPixelSpacing().y;

    // Scale the orthogonal vector with the linewidth
    vec3 j = vec3(orthogonalLine.x * r1, orthogonalLine.y * r2, orthogonalLine.z);

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
