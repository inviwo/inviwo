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
#include "ABufferShading.hglsl"



//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

//Input interpolated fragment position
smooth in vec4 fragPos;
//Output fragment color
out vec4 outFragColor;

//Keeps only closest fragment
vec4 resolveClosest(int pageIdx, int abNumFrag);
//Fill local memory array of fragments
void fillFragmentArray(int pageIdx, int abNumFrag);


//Resolve A-Buffer and blend sorted fragments
void main(void) {

	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	   && coords.x<AbufferParams.screenWidth 
	   && coords.y<AbufferParams.screenHeight ){

		int pageIdx=int(getPixelCurrentPage(coords));

		if(pageIdx > 0 ){

			//Load the number of fragments in the last page.
			int abNumFrag=int(getPixelFragCounter(coords));


			if(abNumFrag>0){

#if ABUFFER_DISPNUMFRAGMENTS==0

				//Copute ans output final color for the frame buffer
#	if ABUFFER_RESOLVE_USE_SORTING==0	
				//If we only want the closest fragment
				outFragColor=resolveClosest(pageIdx, abNumFrag);
#	else
				//Copy fragments in local array
				fillFragmentArray(pageIdx, abNumFrag);
				abNumFrag=min(abNumFrag, ABUFFER_SIZE);
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

			}
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



vec4 resolveClosest(int pageIdx, int abNumFrag){

	int curPage=pageIdx;

	//Search smallest z
	vec4 minFrag=vec4(0.0f, 0.0f, 0.0f, 1000000.0f);
	int ip=0;

	while(curPage!=0 && ip<20){
		int numElem;
		if(ip==0){
			numElem=abNumFrag%(ABUFFER_PAGE_SIZE);
			if(numElem==0)
				numElem=ABUFFER_PAGE_SIZE;
		}else{
			numElem=ABUFFER_PAGE_SIZE;
		}

		for(int i=0; i<numElem; i++){
			vec4 val=sharedPoolGetValue(curPage+i);

			if(val.w<minFrag.w){
				minFrag=val;
			}

		}

		//Get next page index
		curPage=int(sharedPoolGetLink(curPage/ABUFFER_PAGE_SIZE));

		ip++;
	}
	//Output final color for the frame buffer
	return minFrag;
}



void fillFragmentArray(int pageIdx, int abNumFrag){
	//Load fragments into a local memory array for sorting

	int curPage=pageIdx;
	int ip=0;
	int fi=0;
	while(curPage!=0 && ip<20){
		int numElem;
		if(ip==0){
			numElem=abNumFrag%(ABUFFER_PAGE_SIZE);
			if(numElem==0)
				numElem=ABUFFER_PAGE_SIZE;
		}else{
			numElem=ABUFFER_PAGE_SIZE;
		}

		for(int i=0; i<numElem; i++){
			if(fi<ABUFFER_SIZE){
				fragmentList[fi]=sharedPoolGetValue(curPage+i);
			}
			fi++;
		}


		curPage=int(sharedPoolGetLink(curPage/ABUFFER_PAGE_SIZE));

		ip++;
	}

}
