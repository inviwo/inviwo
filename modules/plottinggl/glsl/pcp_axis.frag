#include "pcp_common.glsl"

uniform vec4 color;
uniform vec3 pickColor;

void main() {
    FragData0 = color;
    PickingData = vec4(pickColor, 1);
}
