/**
 * Blends the final color of the illustration buffer
 */

#include "IllustrationBuffer.hglsl"

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
//Input interpolated fragment position
smooth in vec4 fragPos;

void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	    && coords.x<screenSize.x 
	    && coords.y<screenSize.y ){

        uint count = imageLoad(illustrationBufferCountImg, coords).x;
        vec4 color = vec4(0);
        if (count > 0) {
            uint start = imageLoad(illustrationBufferIdxImg, coords).x;
            for (int i=0; i<count; ++i) {
                vec3 baseColor = uncompressColor(illustrationDataIn[i+start].colors);
                float baseAlpha = fract(illustrationDataIn[i+start].alpha);

                color.rgb = color.rgb + (1-color.a)*baseAlpha*baseColor;
                color.a = color.a + (1-color.a)*baseAlpha;
            }
            FragData0 = color;
            return;
        }
    }
    discard;
}
