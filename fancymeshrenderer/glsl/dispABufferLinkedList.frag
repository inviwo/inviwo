/**
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0
 * Copyright Cyril Crassin, July 2010
**/

//#extension GL_NV_gpu_shader5 : enable
//#extension GL_EXT_shader_image_load_store : enable
//#extension GL_NV_shader_buffer_load : enable
//#extension GL_NV_shader_buffer_store : enable
//#extension GL_EXT_bindable_uniform : enable

#include "ABufferLinkedList.hglsl"
#include "ABufferSort.hglsl"

// How should the stuff be rendered? (Debugging options)
#define ABUFFER_DISPNUMFRAGMENTS 0
#define ABUFFER_RESOLVE_USE_SORTING 1

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

//Input interpolated fragment position
smooth in vec4 fragPos;

//Computes only the number of fragments
int getFragmentCount(uint pixelIdx);
//Keeps only closest fragment
vec4 resolveClosest(uint idx);
//Fill local memory array of fragments
void fillFragmentArray(uint idx, out int numFrag);


//Resolve A-Buffer and blend sorted fragments
void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	   && coords.x<AbufferParams.screenWidth 
	   && coords.y<AbufferParams.screenHeight ){

		uint pixelIdx=getPixelLink(coords);

		if(pixelIdx > 0 ){

#if ABUFFER_DISPNUMFRAGMENTS==1
        FragData0=vec4(getFragmentCount(pixelIdx) / float(ABUFFER_SIZE));
#elif ABUFFER_RESOLVE_USE_SORTING==0	
		//If we only want the closest fragment
        vec4 p = resolveClosest(pixelIdx);
        FragData0 = uncompressPixelData(p).color;
#else
		//Copy fragments in local array
        int numFrag = 0;
		fillFragmentArray(pixelIdx, numFrag);
		//Sort fragments in local memory array
		bubbleSort(numFrag);

        //front-to-back shading
		vec4 color = vec4(0);
        for (int i=0; i<numFrag; ++i) {
            vec4 c = uncompressPixelData(fragmentList[i]).color;
            color.rgb = color.rgb + (1-color.a)*c.a*c.rgb;
            color.a = color.a + (1-color.a)*c.a;
        }
        FragData0 = color;
#endif

		}else{ //no pixel found
#if ABUFFER_DISPNUMFRAGMENTS==0
			//If no fragment, write nothing
			discard;
#else
			FragData0=vec4(0.0f);
#endif
		}
	}
}



int getFragmentCount(uint pixelIdx){
    int counter = 0;
    while(pixelIdx!=0 && counter<ABUFFER_SIZE){
        vec4 val = readPixelStorage(pixelIdx-1);
        counter++;
        pixelIdx = floatBitsToUint(val.x);
    }
    return counter;
}

vec4 resolveClosest(uint pixelIdx){

	//Search smallest z
	vec4 minFrag=vec4(0.0f, 1000000.0f, 1.0f, uintBitsToFloat(1024*1023));
	int ip=0;

	while(pixelIdx!=0 && ip<ABUFFER_SIZE){
        vec4 val = readPixelStorage(pixelIdx-1);

        if (val.y<minFrag.y) {
            minFrag = val;
        }

		pixelIdx = floatBitsToUint(val.x);

		ip++;
	}
	//Output final color for the frame buffer
	return minFrag;
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
