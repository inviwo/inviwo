/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_NV_shader_buffer_store : enable
#extension GL_EXT_bindable_uniform : enable

//Macros changed from the C++ side
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512
#define BACKGROUND_COLOR_R 1.0f
#define BACKGROUND_COLOR_G 1.0f
#define BACKGROUND_COLOR_B 1.0f
#define ABUFFER_SIZE 16
#define ABUFFER_USE_TEXTURES 1

#define ABUFFER_RESOLVE_USE_SORTING	1

#define ABUFFER_RESOLVE_ALPHA_CORRECTION 0
#define ABUFFER_RESOLVE_GELLY 0
#define ABUFFER_DISPNUMFRAGMENTS 0


#include "ABufferSort.hglsl"
#include "ABufferShading.hglsl"


//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

//Input interpolated fragment position
smooth in vec4 fragPos;
//Output fragment color
out vec4 outFragColor;

#if ABUFFER_USE_TEXTURES
uniform layout(size1x32) uimage2D abufferCounterImg;
uniform layout(size4x32) image2DArray abufferImg;
#else
uniform vec4 *d_abuffer;
uniform uint *d_abufferIdx;
#endif

//Keeps only closest fragment
vec4 resolveClosest(ivec2 coords, int abNumFrag);
//Fill local memory array of fragments
void fillFragmentArray(ivec2 coords, int abNumFrag);

//Resolve A-Buffer and blend sorted fragments
void main(void) {

	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 && coords.x<SCREEN_WIDTH && coords.y<SCREEN_HEIGHT ){

		//Load the number of fragments in the current pixel.
#if ABUFFER_USE_TEXTURES
		int abNumFrag=(int)imageLoad(abufferCounterImg, coords).r;
#else
		int abNumFrag=(int)d_abufferIdx[coords.x+coords.y*SCREEN_WIDTH];
			
		//Crash without this (WTF ??)
		if(abNumFrag<0 )
			abNumFrag=0;
		if(abNumFrag>ABUFFER_SIZE ){
			abNumFrag=ABUFFER_SIZE;
		}
#endif

		if(abNumFrag > 0){

#if ABUFFER_DISPNUMFRAGMENTS==0

			//Copute ans output final color for the frame buffer
#	if ABUFFER_RESOLVE_USE_SORTING==0	
			//If we only want the closest fragment
			outFragColor=resolveClosest(coords, abNumFrag);
#	else

			//Copy fragments in local array
			fillFragmentArray(coords, abNumFrag);
			//Sort fragments in local memory array
			bubbleSort(abNumFrag);

#		if ABUFFER_RESOLVE_GELLY
			//We want to sort and apply gelly shader
			outFragColor=resolveGelly(abNumFrag);
#		else
			//We want to sort and blend fragments
			outFragColor=resolveAlphaBlend(abNumFrag);
#		endif

#	endif

#else
			outFragColor=vec4(abNumFrag/float(ABUFFER_SIZE));
#endif

		}else{
#if ABUFFER_DISPNUMFRAGMENTS==0
			//If no fragment, write nothing
			discard;
#else
			outFragColor=vec4(0.0f);
#endif
		}

	}
	
}


vec4 resolveClosest(ivec2 coords, int abNumFrag){

	//Search smallest z
	vec4 minFrag=vec4(0.0f, 0.0f, 0.0f, 1000000.0f);
	for(int i=0; i<abNumFrag; i++){
#	if ABUFFER_USE_TEXTURES
		vec4 val=imageLoad(abufferImg, ivec3(coords, i));
#	else
		vec4 val=d_abuffer[coords.x+coords.y*SCREEN_WIDTH + (i*SCREEN_WIDTH*SCREEN_HEIGHT)];
#	endif
		if(val.w<minFrag.w){
			minFrag=val;
		}
	}

	//Output final color for the frame buffer
	return minFrag;
}


void fillFragmentArray(ivec2 coords, int abNumFrag){
	//Load fragments into a local memory array for sorting
	for(int i=0; i<abNumFrag; i++){
#	if ABUFFER_USE_TEXTURES
		fragmentList[i]=imageLoad(abufferImg, ivec3(coords, i));
#	else
		fragmentList[i]=d_abuffer[coords.x+coords.y*SCREEN_WIDTH + (i*SCREEN_WIDTH*SCREEN_HEIGHT)];
#	endif
	}
}