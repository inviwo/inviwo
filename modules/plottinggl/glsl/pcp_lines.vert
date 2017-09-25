#include "pcp_common.glsl"

out vec3 pickColor;
out vec2 texCoord;


void main() {
    pickColor = in_Normal.rgb;
    texCoord = in_TexCoord.xy;
    gl_Position = vec4(getPosWithSpacing(vec2(in_Vertex.xy)), 0, 1);
}