#include "pcp_common.glsl"

uniform float x;
uniform float y;
uniform int flipped;

uniform float w = 40;
uniform float h = 20;

out vec2 p;

void main() {
    float w2 = w / (dims.x) / 2;
    float h2 = h / (dims.y);

    if (flipped == 0) {
        h2 *= -1;
    }

    if (gl_VertexID == 0) {
        p = vec2(0, 0);
        gl_Position = vec4(getPosWithSpacing(vec2(x - w2, y)), 0, 1);
    } else if (gl_VertexID == 1) {
        p = vec2(0, 1);
        gl_Position = vec4(getPosWithSpacing(vec2(x - w2, y + h2)), 0, 1);
    } else if (gl_VertexID == 2) {
        p = vec2(1, 0);
        gl_Position = vec4(getPosWithSpacing(vec2(x + w2, y)), 0, 1);
    } else if (gl_VertexID == 3) {
        p = vec2(1, 1);
        gl_Position = vec4(getPosWithSpacing(vec2(x + w2, y + h2)), 0, 1);
    }
}
