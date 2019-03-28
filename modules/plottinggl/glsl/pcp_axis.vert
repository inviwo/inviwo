#include "pcp_common.glsl"

uniform float x;

out vec2 texCoord;

void main() {
    texCoord = in_TexCoord.xy;
    gl_Position = vec4(getPosWithSpacing(vec2(x, in_Vertex.y)), 0, 1);
}
