/**
 * Fast Single-pass A-Buffer using OpenGL 4.0  V2.0
 * Copyright Cyril Crassin, July 2010
**/

//#extension GL_NV_gpu_shader5 : enable
//#extension GL_EXT_shader_image_load_store : enable
//#extension GL_NV_shader_buffer_load : enable
//#extension GL_EXT_bindable_uniform : enable
//#extension GL_NV_shader_buffer_store : enable

#include "ABufferLinkedList.hglsl"

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;


void main(void) {

	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	   && coords.x<AbufferParams.screenWidth 
	   && coords.y<AbufferParams.screenHeight ){

		imageStore(abufferIdxImg, coords, ivec4(0));

		//sharedPoolSetLink(0, 0U);
		//sharedPoolSetValue(0, vec4(0.0f, 0.0f, 4.0f, 1.0f));
		
	}

	//Discard fragment so nothing is writen to the framebuffer
	discard;
}


