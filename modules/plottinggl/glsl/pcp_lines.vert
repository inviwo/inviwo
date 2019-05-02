#include "pcp_common.glsl"

out float vScalarMeta;
flat out uint vPicking;

uniform float axisPositions[NUMBER_OF_AXIS];
uniform bool axisFlipped[NUMBER_OF_AXIS];

void main() {
    vScalarMeta = in_ScalarMeta;
    vPicking = in_Picking;

    int axisIndex = gl_VertexID % NUMBER_OF_AXIS;

    float xPos = axisPositions[axisIndex];
    float yPos = mix(in_Vertex, 1.0 - in_Vertex, axisFlipped[axisIndex]);
    gl_Position = vec4(getPosWithSpacing(vec2(xPos, yPos)), 0.0, 1.0);
}
