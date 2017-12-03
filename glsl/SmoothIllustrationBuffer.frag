/*
 * Smoothes the silhouette and halo borders
 */

#include "IllustrationBuffer.hglsl"

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
//Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding=0) buffer neighborBufferIn
{
    ivec4 neighborsIn[];
};
layout(std430, binding=1) buffer smoothingBufferIn
{
    vec2 smoothingIn[]; //beta + gamma
};
layout(std430, binding=2) buffer smoothingBufferOut
{
    vec2 smoothingOut[]; //beta + gamma
};

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
                ivec4 neighbors = neighborsIn[start + i];
                vec2 smoothing = smoothingIn[start + i];
                //smooth beta;
                float beta = smoothing.x;
                for (int j=0; j<4; ++j) {
                    if (neighbors[j]>=0) beta = max(beta, smoothingIn[neighbors[j]].x * (1-lambdaBeta));
                }
                smoothing.x = beta;
                //smooth gamma;
                float gamma = smoothing.y;
                for (int j=0; j<4; ++j) {
                    if (neighbors[j]>=0) gamma = max(gamma, smoothingIn[neighbors[j]].y * (1-lambdaGamma));
                }
                smoothing.y = gamma;
                //write back
                smoothingOut[start + i] = smoothing;
            }
        }
    }
    discard;
}