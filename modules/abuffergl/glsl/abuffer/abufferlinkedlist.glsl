/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef ABUFFERLINKEDLIST_GLSL
#define ABUFFERLINKEDLIST_GLSL

/*
//Required Macros (Examples)
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512
#define BACKGROUND_COLOR_R 0.0f
#define BACKGROUND_COLOR_G 0.0f
#define BACKGROUND_COLOR_B 0.0f
#define ABUFFER_SIZE 16
#define ABUFFER_PAGE_SIZE 4
#define ABUFFER_RGBA_DATA_TYPE_SIZE sizeof(u8vec4)
#define ABUFFER_EXT_DATA_TYPE_SIZE sizeof(float32)
#define ABUFFER_DATA_PER_NODE 1
#define ABUFFER_USE_TEXTURES 1        //( always 1)
#define SHAREDPOOL_USE_TEXTURES 0     //( always 0)
#define ABUFFER_RESOLVE_USE_SORTING	1 //( always 1)
#define ABUFFER_DISPNUMFRAGMENTS 0
*/

uniform int abufferSharedPoolSize;

#if ABUFFER_USE_TEXTURES
//A-Buffer page counter
coherent uniform layout(r32ui) uimage2D abufferPageIdxImg;
//A-Buffer fragment counter
coherent uniform layout(r32ui) uimage2D abufferFragCountImg;
//A-Buffer semaphore lock-release
coherent uniform layout(r32ui) uimage2D semaphoreImg;
#else
coherent uniform uint *d_abufferPageIdx;
coherent uniform uint *d_abufferFragCount;
coherent uniform uint *d_semaphore;
#endif

#if SHAREDPOOL_USE_TEXTURES
//A-Buffer shared fragment data
coherent uniform layout(rgbau8i) uimageBuffer shared_RGBA_DataPageListImg; //rgb data
coherent uniform layout(r32f) imageBuffer shared_EXT_DataPageListImg; //ext data

//A-Buffer shared link list
coherent uniform layout(r32ui) uimageBuffer sharedLinkListImg;
#else
coherent uniform u8vec4 *d_shared_RGBA_DataPageList;
coherent uniform float *d_shared_Ext_DataPageList;
coherent uniform uint *d_sharedLinkList;
#endif


//Next available page in the shared pool
coherent uniform uint *d_curSharedPage;

//Global atomic counter generating page indices
layout (binding = 0, offset = 0) uniform atomic_uint atomicPageCounter;


#if ABUFFER_USE_TEXTURES

bool semaphoreAcquire(ivec2 coords){
	return imageAtomicExchange(semaphoreImg, coords, 1U)==0U;
}

void semaphoreRelease(ivec2 coords){
	imageAtomicExchange(semaphoreImg, coords, 0U);
}

bool getSemaphore(ivec2 coords){
	return imageLoad(semaphoreImg, coords).x==0U;
}
void setSemaphore(ivec2 coords, bool val){
	imageStore(semaphoreImg, coords, uvec4(val ? 1U : 0U, 0U, 0U, 0U));	
	memoryBarrier();
}

uint getPixelCurrentPage(ivec2 coords){
	return imageLoad(abufferPageIdxImg, coords).x;
}
void setPixelCurrentPage(ivec2 coords, uint newpageidx){
	imageStore(abufferPageIdxImg, coords, uvec4(newpageidx, 0U, 0U, 0U) );	
	memoryBarrier();
}

uint getPixelFragCounter(ivec2 coords){
	return imageLoad(abufferFragCountImg, coords).x;
}

void setPixelFragCounter(ivec2 coords, uint val){
	imageStore(abufferFragCountImg, coords, uvec4(val, 0U, 0U, 0U) );
	memoryBarrier();
}

uint pixelFragCounterAtomicAdd(ivec2 coords, uint val){
	return imageAtomicAdd(abufferFragCountImg, coords, val);
}

#else

//semaphores
bool semaphoreAcquire(ivec2 coords) {
	return atomicExchange(d_semaphore+coords.x+coords.y*SCREEN_WIDTH, 0U) != 0U;
}

void semaphoreRelease(ivec2 coords) {
	atomicExchange(d_semaphore+coords.x+coords.y*SCREEN_WIDTH, 1U);
}

bool getSemaphore(ivec2 coords) {
	return d_semaphore[coords.x+coords.y*SCREEN_WIDTH]==0U;
}

void setSemaphore(ivec2 coords, bool val) {
	d_semaphore[coords.x+coords.y*SCREEN_WIDTH]=val ? 0U : 1U;
}

//page counter
uint getPixelCurrentPage(ivec2 coords) {
	return d_abufferPageIdx[coords.x+coords.y*SCREEN_WIDTH];
}

void setPixelCurrentPage(ivec2 coords, uint newpageidx) {
	d_abufferPageIdx[coords.x+coords.y*SCREEN_WIDTH]=newpageidx;
}

//pixel counter
uint getPixelFragCounter(ivec2 coords) {
	return d_abufferFragCount[coords.x+coords.y*SCREEN_WIDTH];
}

void setPixelFragCounter(ivec2 coords, uint val) {
	d_abufferFragCount[coords.x+coords.y*SCREEN_WIDTH]=val;
}

uint pixelFragCounterAtomicAdd(ivec2 coords, uint val) {
	return atomicAdd(d_abufferFragCount+coords.x+coords.y*SCREEN_WIDTH, val);
}

#endif


#if SHAREDPOOL_USE_TEXTURES

uint sharedPoolGetLink(uint pageNum) {
	return imageLoad(sharedLinkListImg, (int)(pageNum) ).x;
}

void sharedPoolSetLink(uint pageNum, uint pointer) {
	imageStore(sharedLinkListImg, (int)(pageNum), uvec4(pointer, 0U, 0U, 0U) );
	memoryBarrier();
}

void sharedPool_GetRGBAValue(uint index, out u8vec4 val){
	val = imageLoad(shared_RGBA_DataPageListImg, (int)(index*ABUFFER_DATA_PER_NODE));
}

void sharedPool_GetExtValue(uint index, out float val){
	val = imageLoad(shared_Ext_DataPageListImg, (int)(index*ABUFFER_DATA_PER_NODE));
}

void sharedPool_SetRGBAValue(uint index, u8vec4 val){
	imageStore(shared_RGBA_DataPageListImg, (int)(index*ABUFFER_DATA_PER_NODE), val);
	memoryBarrier(); 
}

void sharedPool_SetExtValue(uint index, float val){
	imageStore(shared_Ext_DataPageListImg, (int)(index*ABUFFER_DATA_PER_NODE), val);
	memoryBarrier(); 
}

//Set-Get functions
void sharedPool_GetValue(uint index, out u8vec4 val1, out float val2){
	sharedPool_GetRGBAValue(index, val1);
	sharedPool_GetExtValue(index, val2);
}

void sharedPool_SetExtValue(uint index, u8vec4 val1, float val2){
	sharedPool_SetRGBAValue(index, val1);
	sharedPool_SetExtValue(index, val2); 
}

#else

uint sharedPoolGetLink(uint pageNum) {
	return d_sharedLinkList[(int)pageNum];
}

void sharedPoolSetLink(uint pageIdx, uint pointer) {
	d_sharedLinkList[(int)pageIdx]=pointer;
}

void sharedPool_GetRGBAValue(uint index, out u8vec4 val) {
	val = d_shared_RGBA_DataPageList[((int)index)*ABUFFER_DATA_PER_NODE];
}

void sharedPool_GetExtValue(uint index, out float val) {
	val = d_shared_Ext_DataPageList[((int)index)*ABUFFER_DATA_PER_NODE];
}

void sharedPool_SetRGBAValue(uint index, u8vec4 val) {
	d_shared_RGBA_DataPageList[((int)index)*ABUFFER_DATA_PER_NODE]=val;
}

void sharedPool_SetExtValue(uint index, float val) {
	d_shared_Ext_DataPageList[((int)index)*ABUFFER_DATA_PER_NODE]=val;
}

//Set-Get functions
void sharedPoolSetValue(uint index, u8vec4 val1, float val2) {
	sharedPool_SetRGBAValue(index, val1);
	sharedPool_SetExtValue(index, val2);
}

void sharedPoolGetValue(uint index, out u8vec4 val1, out float val2) {
	sharedPool_GetRGBAValue(index, val1);
	sharedPool_GetExtValue(index, val2);
}

#endif

void setSharedPageCounter(uint val) {
	(*d_curSharedPage)=val;
}

uint sharedPageCounterAtomicAdd(/*uint val*/) {	
	return atomicCounterIncrement(atomicPageCounter);
	//this atomicAdd did not work hence atomic increment is used
	// return atomicAdd(d_curSharedPage, val); 
}

uint getSharedPageCounterAtomicValue() {
	return (*d_curSharedPage);
}

#endif	//ABUFFERLINKEDLIST_GLSL