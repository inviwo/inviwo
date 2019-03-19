#include "pcp_common.glsl"

out float vScalarMeta;
flat out uint vPicking;

uniform float axisPositions[NUMBER_OF_AXIS];

void main() {
    vScalarMeta = in_ScalarMeta;
    vPicking = in_Picking;

    float xPos = axisPositions[gl_VertexID % NUMBER_OF_AXIS];   
    gl_Position = vec4(getPosWithSpacing(vec2(xPos, in_Vertex)), 0.0, 1.0);
}