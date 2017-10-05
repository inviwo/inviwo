/**
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0
 * Copyright Cyril Crassin, July 2010
**/

#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_EXT_bindable_uniform : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_NV_shader_buffer_store : enable

#include "ABufferLinkedList.hglsl"

//Whole number pixel offsets (not necessary, just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

smooth in vec4 fragPos;
smooth in vec3 fragTexCoord;
smooth in vec3 fragNormal;


//Shade using green-white strips
vec3 shadeStrips(vec3 texcoord){
	vec3 col;
	float i=floor(texcoord.x*6.0f);

	col.rgb=fract(i*0.5f) == 0.0f ? vec3(0.4f, 0.85f, 0.0f) : vec3(1.0f);
	col.rgb*=texcoord.z;

	return col;
}

#if USE_ABUFFER

void main(void) {

	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	//Check we are in the framebuffer
	if(coords.x>=0 && coords.y>=0 && coords.x<SCREEN_WIDTH && coords.y<SCREEN_HEIGHT ){

		//Get current page
		uint curPage=0;
		uint curFragIdx=0;

		///Manual critical section => This can most probably be optimized !
		int ii=0; //prevents infinite loops
		bool leaveLoop = false;
		while (!leaveLoop && ii<1000) {

			//Aquire semaphore
			if ( semaphoreAcquire(coords) ) {

				//Get current page
				curPage=getPixelCurrentPage(coords);

				//Get fragment counter
				curFragIdx=getPixelFragCounter(coords);
				uint curFragIdxMod=curFragIdx%ABUFFER_PAGE_SIZE;

				if(curFragIdxMod==0 ){
					uint newpageidx=sharedPageCounterAtomicAdd(ABUFFER_PAGE_SIZE);

					if(newpageidx<abufferSharedPoolSize){
						//Create link
						sharedPoolSetLink(newpageidx/ABUFFER_PAGE_SIZE, curPage);

						//Save new page idx
						setPixelCurrentPage(coords, newpageidx);

						curPage=newpageidx;
					}else{
						curPage=0;
					}
				}

				if(curPage>0){
					setPixelFragCounter(coords, curFragIdx+1);
				}

				curFragIdx=curFragIdxMod;

				leaveLoop = true;

				//memoryBarrier(); //We theoretically need it here, but not implemented in current drivers !
				semaphoreRelease(coords);
			}

			ii++;
		} 


		//Create fragment to be stored
		vec4 abuffval;
		vec3 col;

		//Choose what we store per fragment
#if ABUFFER_RESOLVE_GELLY==0
		//Store color strips
		col=shadeStrips(fragTexCoord);
#else
		//Store pseudo-illumination info
		vec3 N = normalize(fragNormal);
		vec3 L = normalize(vec3(0.0f,1.0f,1.0f));
		col = vec3(dot(N,L));
#endif

		abuffval.rgb=col;
		abuffval.w=fragPos.z;	//Will be used for sorting

		//Put fragment into the  page in the shared pool
		sharedPoolSetValue(curPage+curFragIdx, abuffval);

		//Do not discard the fragment if a page failled to be allocated
		if(curPage!=0)
			discard;
	}else{
		discard;
	}


}

#else	//#if USE_ABUFFER

out vec4 outFragColor;

void main(void) {
	vec3 col=shadeStrips(fragTexCoord);
	outFragColor=vec4(col, 1.0f);
}

#endif	//#if USE_ABUFFER