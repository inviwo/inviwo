/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_EXT_bindable_uniform : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_NV_shader_buffer_store : enable

//Macros changed from the C++ side
#define SCREEN_WIDTH	512
#define SCREEN_HEIGHT	512
#define USE_ABUFFER	1
#define ABUFFER_SIZE 16
#define ABUFFER_USE_TEXTURES 1
#define ABUFFER_RESOLVE_GELLY 0

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

smooth in vec4 fragPos;
smooth in vec3 fragTexCoord;

smooth in vec3 fragNormal;

#if ABUFFER_USE_TEXTURES
//A-Buffer fragments storage
coherent uniform layout(size4x32) image2DArray abufferImg;
//A-Buffer fragment counter
coherent uniform layout(size1x32) uimage2D abufferCounterImg;

#else
uniform vec4 *d_abuffer;
uniform coherent uint *d_abufferIdx;
#endif


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

		//Atomic increment of the counter
#if ABUFFER_USE_TEXTURES==0
		int abidx=(int)atomicIncWrap(d_abufferIdx+coords.x+coords.y*SCREEN_WIDTH, ABUFFER_SIZE);
#else

		int abidx=(int)imageAtomicIncWrap(abufferCounterImg, coords, ABUFFER_SIZE );
		//int abidx=imageAtomicAdd(abufferCounterImg, coords, 1);

#endif

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

		//Put fragment into A-Buffer
#if ABUFFER_USE_TEXTURES==0
		d_abuffer[coords.x+coords.y*SCREEN_WIDTH + (abidx*SCREEN_WIDTH*SCREEN_HEIGHT)]=abuffval;
#else
		imageStore(abufferImg, ivec3(coords, abidx), abuffval);
#endif
	

	}

	//Discard fragment so nothing is writen to the framebuffer
	discard;
}

#else	//#if USE_ABUFFER

out vec4 outFragColor;

void main(void) {
	vec3 col=shadeStrips(fragTexCoord);
	outFragColor=vec4(col, 1.0f);
}

#endif	//#if USE_ABUFFER