/*
 * Smoothes the silhouette and halo borders
 */

#include "IllustrationBuffer.hglsl"

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
//Input interpolated fragment position
smooth in vec4 fragPos;

//diffusion coefficient
uniform float lambdaBeta = 0.4;
uniform float lambdaGamma = 0.1;

void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	    && coords.x<screenSize.x 
	    && coords.y<screenSize.y ){

        int count = int(imageLoad(illustrationBufferCountImg, coords).x);
        if (count > 0) {
            int start = int(imageLoad(illustrationBufferIdxImg, coords).x);
            for (int i=0; i<count; ++i) {
                FragmentData d = illustrationDataIn[start + i];
                //smooth beta;
                float beta = d.silhouetteHighlight;
                for (int j=0; j<4; ++j) {
                    if (d.neighbors[j]>=0) beta = max(beta, illustrationDataIn[d.neighbors[j]].silhouetteHighlight - lambdaBeta);
                }
                d.silhouetteHighlight = beta;
                //smooth gamma;
                float gamma = d.haloHighlight;
                for (int j=0; j<4; ++j) {
                    if (d.neighbors[j]>=0) gamma = max(gamma, illustrationDataIn[d.neighbors[j]].haloHighlight - lambdaGamma);
                }
                d.haloHighlight = gamma;
                //write back
                illustrationDataOut[start + i] = d;
            }
        }
    }
    discard;
}