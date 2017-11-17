/**
 * Convertion from the a-buffer (fragment lists) to the illustration buffers:
 * Load fragments, sort them, store them in linear memory
 */

#include "ABufferLinkedList.hglsl"
#include "IllustrationBuffer.hglsl"
#include "ABufferSort.hglsl"

//atomic counter to allocate space in the illustration buffer
//just reuse the counter from the A-Buffer
#define illustrationBufferCounter abufferCounter

//We need some way to add an arbitrary count to the atomic buffer, not just increment it
#if defined(GLSL_VERSION_460)
#define atomicAdd atomicCounterAdd
#elif defined(GL_ARB_shader_atomic_counter_ops)
#define atomicAdd atomicCounterAddARB
#else
#error "Sorry, but to fill the Illustration Buffer, I need to be able to add an atomic counter: OpenGL 4.6 or ARB_shader_atomic_counter_ops"
#endif


//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

//Input interpolated fragment position
smooth in vec4 fragPos;

//Fill local memory array of fragments
void fillFragmentArray(uint idx, out int numFrag);

void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	    && coords.x<AbufferParams.screenWidth 
	    && coords.y<AbufferParams.screenHeight ){

        uint pixelIdx=getPixelLink(coords);
        if (pixelIdx > 0)
        {
            //we have fragments
            //1. load them
            int numFrag = 0;
		    fillFragmentArray(pixelIdx, numFrag);
		    //2. sort fragments in local memory array
		    bubbleSort(numFrag);
            //3. write them back
            uint start = atomicAdd(illustrationBufferCounter, numFrag);
            for (int i=0; i<numFrag; ++i) {
                FragmentData data;
                data.depth = fragmentList[i].y;
                data.depthGradient = 0; //to be filled out
                data.alpha = fragmentList[i].z + 2;
                data.colors = floatBitsToUint(fragmentList[i].w);
                data.neighbors = ivec4(-1);
                data.silhouetteHighlight = 2;
                data.haloHighlight = 2;
                data.index = i;
                data.dummy1 = 0;
                illustrationDataOut[start + i] = data;
            }
            imageStore(illustrationBufferIdxImg, coords, ivec4(start));
            imageStore(illustrationBufferCountImg, coords, ivec4(numFrag));
        }
        else
        {
            //no fragments, clear texture
            imageStore(illustrationBufferIdxImg, coords, ivec4(0));
            imageStore(illustrationBufferCountImg, coords, ivec4(0));
        }
    }
    discard;
}

void fillFragmentArray(uint pixelIdx, out int numFrag){
	//Load fragments into a local memory array for sorting

	int ip=0;
	while(pixelIdx!=0 && ip<ABUFFER_SIZE){
		vec4 val = readPixelStorage(pixelIdx-1);
        fragmentList[ip] = val;
        pixelIdx = floatBitsToUint(val.x);
		ip++;
	}
    numFrag = ip;
}