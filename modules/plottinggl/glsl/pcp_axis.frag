#include "pcp_common.glsl"

in float axisXpos;

uniform vec4 color;
uniform vec4 hoverColor;
uniform vec4 selectedColor;
uniform vec3 pickColor;
uniform int hover;
uniform int selected;

void main() {

    float axisPixelPosition = (axisXpos + 1.0) * 0.5 * dims.x;
    float distanceToAxis = abs(gl_FragCoord.x - axisPixelPosition);

	float axisWidth = 2;
	float axisWidthHover = 4;
	float axisWidthSelected = 6;

    vec4 res = vec4(0);
    if (hover == 1 && distanceToAxis < axisWidthHover * 0.5) {
        res = hoverColor;
    } else if (selected == 1 && distanceToAxis < axisWidthSelected * 0.5) {
        res = selectedColor;
    } else if (distanceToAxis < axisWidth * 0.5) {
        res = color;
    }

    FragData0 = res;
    PickingData = vec4(pickColor, 1);
}
