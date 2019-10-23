/**
 * Blends the final color of the illustration buffer
 */

#include "IllustrationBuffer.hglsl"

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
//Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding=0) buffer colorBufferIn
{
    vec2 colorIn[]; //alpha+rgb
};
layout(std430, binding=1) buffer smoothingBufferIn
{
    vec2 smoothingIn[]; //beta + gamma
};

uniform vec4 edgeColor = vec4(0, 0, 0, 1);
uniform float haloStrength = 0.4;

void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	    && coords.x<screenSize.x 
	    && coords.y<screenSize.y ){

        uint count = imageLoad(illustrationBufferCountImg, coords).x;
        vec4 color = vec4(0,0,0,0);
        if (count > 0) {
            uint start = imageLoad(illustrationBufferIdxImg, coords).x;
            for (int i=0; i<count; ++i) {
                //fetch properties from the scalar fields
                vec3 baseColor = uncompressColor(floatBitsToInt(colorIn[i+start].y));
                float alpha = colorIn[i+start].x;
                alpha = clamp(alpha, 0, 1);
                float beta = smoothingIn[i+start].x;
                float gamma = smoothingIn[i+start].y;

                //blend them together
                float alphaHat = (1-gamma*haloStrength*(1-beta))*(alpha+(1-alpha)*beta*edgeColor.a);
                baseColor = mix(baseColor, edgeColor.rgb, beta*edgeColor.a);
                color.rgb = color.rgb + (1-color.a)*alphaHat*baseColor;
                color.a = color.a + (1-color.a)*alphaHat;
            }
            FragData0 = color;
            return;
        }
    }
    discard;
}
