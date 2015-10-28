/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/abuffergl/abufferglhelpers/abuffergllinkedlist.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

Inviwo_ABufferGL4_LinkedList::Inviwo_ABufferGL4_LinkedList(ivec2 dim) : Inviwo_ABufferGL4(dim, ABuffer_LinkedList),
        abufferPageIdxImgTexture_(0),
        abufferFragCountImgTexture_(0),
        semaphoreImgTexture_(0),
        sharedPoolSize_(dim.x*dim.y * 10 * ABUFFER_PAGE_SIZE*ABUFFER_DATA_PER_NODE),
        abufferSize_(static_cast<int>(sharedPoolSize_)), // TODO: why using int here?
        totalFragmentQuery_(0),
        lastFrameNumFrags_(0),
        queryRequested_(false),
        curSharedPageBuffID_(0),
        curSharedPageAddress_(0),
        abufferPageIdxBuffID_(0),
        abufferPageIdxAddress_(0),
        abufferPageIdxTexID_(0),
        abufferFragCountBuffID_(0),
        abufferFragCountAddress_(0),
        abufferFragCountTexID_(0),
        shared_RGBA_DataListBuffID_(0),
        shared_RGBA_DataListAddress_(0),
        shared_RGBA_DataListTexID_(0),
        shared_Ext_DataListBuffID_(0),
        shared_Ext_DataListAddress_(0),
        shared_Ext_DataListTexID_(0),
        sharedPageListComparisonBuffID_(0),
        sharedPageListComparisonAddress_(0),
        sharedPageListComparisonTexID_(0),
        sharedLinkListBuffID_(0),
        sharedLinkListAddress_(0),
        sharedLinkListTexID_(0),
        semaphoreBuffID_(0),
        semaphoreAddress_(0),
        semaphoreTexID_(0),
        globalAtomicCounterBuffer_(0),
        globalAtomicsBufferId_(0)
    {}

Inviwo_ABufferGL4_LinkedList::~Inviwo_ABufferGL4_LinkedList() {
    deinitABuffer();
}

void Inviwo_ABufferGL4_LinkedList::allocateABuffer() {

    abufferPageIdxImgTexture_ = new Image(dim_,  DataUInt32::get());
   

    abufferFragCountImgTexture_ = new Image(dim_, DataUInt32::get());
    
    semaphoreImgTexture_ = new Image(dim_, DataUInt32::get());    

    curSharedPageAddress_=ABUFFER_PAGE_SIZE;

    //Shared page global counter
    {
        size_t headerCounterSize_ = 4;
        globalAtomicCounterBuffer_ = new GLuint[headerCounterSize_];
        memset(globalAtomicCounterBuffer_, 0, headerCounterSize_*sizeof(GLuint) );        
    }
}

void Inviwo_ABufferGL4_LinkedList::initABuffer(int abufferSize) {

    abufferSize_ = abufferSize;
    Layer* layer = abufferPageIdxImgTexture_->getColorLayer();    
    memset(layer->getEditableRepresentation<LayerRAM>()->getData(), 0, dim_.x*dim_.y*sizeof(glm::uint32) );
    //layer->getEditableRepresentation<LayerGL>();

    layer = abufferFragCountImgTexture_->getColorLayer();
    memset(layer->getEditableRepresentation<LayerRAM>()->getData(), 0, dim_.x*dim_.y*sizeof(glm::uint32) );
    //layer->getEditableRepresentation<LayerGL>();

    layer = semaphoreImgTexture_->getColorLayer();
    memset(layer->getEditableRepresentation<LayerRAM>()->getData(), 0, dim_.x*dim_.y*sizeof(glm::uint32) );
    //layer->getEditableRepresentation<LayerGL>();

    initABuffer_LinkedList();

    abufferSize_ = 0;
}


void Inviwo_ABufferGL4_LinkedList::deinitABuffer(void) {
    if (abufferPageIdxImgTexture_) delete abufferPageIdxImgTexture_;
    if (abufferFragCountImgTexture_) delete abufferFragCountImgTexture_;
    if (semaphoreImgTexture_) delete semaphoreImgTexture_;
    if (globalAtomicCounterBuffer_) delete globalAtomicCounterBuffer_;
    clear();

	//TODO:This buffer should be deleted
    if(shared_RGBA_DataListBuffID_) {       
        glBindBuffer(GL_TEXTURE_BUFFER, shared_RGBA_DataListBuffID_);
        glBufferData(GL_TEXTURE_BUFFER, 1, NULL, GL_STATIC_DRAW);
        glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
        glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &shared_RGBA_DataListAddress_); 
    }

    if(shared_Ext_DataListBuffID_) {       
        glBindBuffer(GL_TEXTURE_BUFFER, shared_Ext_DataListBuffID_);
        glBufferData(GL_TEXTURE_BUFFER, 1, NULL, GL_STATIC_DRAW);
        glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
        glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &shared_Ext_DataListAddress_); 
    }

    if(sharedLinkListBuffID_) {       
        glBindBuffer(GL_TEXTURE_BUFFER, sharedLinkListBuffID_);
        glBufferData(GL_TEXTURE_BUFFER, (1)*sizeof(GLuint64), NULL, GL_STATIC_DRAW);
        glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
        glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &sharedLinkListAddress_); 
    }

    curSharedPageBuffID_ = 0;
    abufferPageIdxBuffID_= 0;
    abufferFragCountBuffID_= 0;
    shared_RGBA_DataListBuffID_= 0;
    sharedPageListComparisonBuffID_= 0;
    sharedLinkListBuffID_= 0;
    semaphoreBuffID_= 0;
    globalAtomicsBufferId_= 0;
}

void Inviwo_ABufferGL4_LinkedList::clear(void) {
    for (size_t i=0; i<texUnits_.size(); i++)
        delete texUnits_[i];
    texUnits_.clear();
}

void Inviwo_ABufferGL4_LinkedList::initSharedPool_LinkedList() {

    size_t bufferSize = sharedPoolSize_;
    if (sharedPoolSize_<abufferSize_) bufferSize = abufferSize_;

    if( pSharedPoolUseTextures_ ){
        TextureUnit* texUnit=0;
		///Shared page list/// - RGBA data
		if(!shared_RGBA_DataListBuffID_)
			glGenBuffers(1, &shared_RGBA_DataListBuffID_);
		glBindBuffer(GL_TEXTURE_BUFFER, shared_RGBA_DataListBuffID_);
		glBufferData(GL_TEXTURE_BUFFER, bufferSize*ABUFFER_RGBA_DATA_TYPE_SIZE, NULL, GL_STATIC_DRAW);
		glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &shared_RGBA_DataListAddress_); 

        texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);

		if(!shared_RGBA_DataListTexID_)
			glGenTextures(1, &shared_RGBA_DataListTexID_);
		texUnit->activate();
		glBindTexture(GL_TEXTURE_BUFFER, shared_RGBA_DataListTexID_);
		//Associate BO storage with the texture
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, shared_RGBA_DataListBuffID_);
		glBindImageTextureEXT(texUnit->getUnitNumber(), shared_RGBA_DataListTexID_, 0, false, 0,  GL_READ_WRITE, GL_RGBA8UI);


        ///Shared page list/// - Ext data
        if(!shared_Ext_DataListBuffID_)
            glGenBuffers(1, &shared_Ext_DataListBuffID_);
        glBindBuffer(GL_TEXTURE_BUFFER, shared_Ext_DataListBuffID_);
        glBufferData(GL_TEXTURE_BUFFER, bufferSize*ABUFFER_EXT_DATA_TYPE_SIZE, NULL, GL_STATIC_DRAW);
        glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
        glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &shared_Ext_DataListAddress_); 

        texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);

        if(!shared_Ext_DataListTexID_)
            glGenTextures(1, &shared_Ext_DataListTexID_);
        texUnit->activate();
        glBindTexture(GL_TEXTURE_BUFFER, shared_Ext_DataListTexID_);
        //Associate BO storage with the texture
        glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, shared_Ext_DataListBuffID_);
        glBindImageTextureEXT(texUnit->getUnitNumber(), shared_Ext_DataListTexID_, 0, false, 0,  GL_READ_WRITE, GL_R32F);


        /*
        ///Shared page list/// - comparison values for sorting link list eg, depth
        if(!sharedPageListComparisonBuffID_)
            glGenBuffers(1, &sharedPageListComparisonBuffID_);
        glBindBuffer(GL_TEXTURE_BUFFER, sharedPageListComparisonBuffID_);
        glBufferData(GL_TEXTURE_BUFFER, sharedPoolSize_*ABUFFER_DATA_TYPE_SIZE, NULL, GL_STATIC_DRAW);
        glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
        glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &sharedPageListComparisonAddress_); 

        texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);

        if(!sharedPageListComparisonTexID_)
            glGenTextures(1, &sharedPageListComparisonTexID_);
        texUnit->activate();
        glBindTexture(GL_TEXTURE_BUFFER, sharedPageListComparisonTexID_);
        //Associate BO storage with the texture
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, sharedPageListComparisonBuffID_);
        glBindImageTextureEXT(texUnit->getUnitNumber(), sharedPageListComparisonTexID_, 0, false, 0,  GL_READ_WRITE, GL_RGBA32F);
		*/


		///ABuffer shared link pointer list///
		if(!sharedLinkListBuffID_)
			glGenBuffers(1, &sharedLinkListBuffID_);
		glBindBuffer(GL_TEXTURE_BUFFER, sharedLinkListBuffID_);
		glBufferData(GL_TEXTURE_BUFFER, (bufferSize/ABUFFER_PAGE_SIZE)*sizeof(glm::uint32), NULL, GL_STATIC_DRAW);
		glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &sharedLinkListAddress_); 

        texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);

		if(!sharedLinkListTexID_)
			glGenTextures(1, &sharedLinkListTexID_);
		texUnit->activate();
		glBindTexture(GL_TEXTURE_BUFFER, sharedLinkListTexID_);
		//Associate BO storage with the texture
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, sharedLinkListBuffID_);
		glBindImageTextureEXT(texUnit->getUnitNumber(), sharedLinkListTexID_, 0, false, 0,  GL_READ_WRITE, GL_R32UI);
		
	}else{        
        
		///Shared page list///- data
		if(!shared_RGBA_DataListBuffID_) {
			glGenBuffers(1, &shared_RGBA_DataListBuffID_);
            glBindBuffer(GL_TEXTURE_BUFFER, shared_RGBA_DataListBuffID_);
            glBufferData(GL_TEXTURE_BUFFER, bufferSize*ABUFFER_RGBA_DATA_TYPE_SIZE, NULL, GL_STATIC_DRAW);
            glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
            glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &shared_RGBA_DataListAddress_); 
        }

        if(!shared_Ext_DataListBuffID_) {
            glGenBuffers(1, &shared_Ext_DataListBuffID_);
            glBindBuffer(GL_TEXTURE_BUFFER, shared_Ext_DataListBuffID_);
            glBufferData(GL_TEXTURE_BUFFER, bufferSize*ABUFFER_EXT_DATA_TYPE_SIZE, NULL, GL_STATIC_DRAW);
            glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
            glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &shared_Ext_DataListAddress_); 
        }

        //LogInfo("d_sharedPageList (Data) -> VBO "<< sharedPageListBuffID_)

        ///Shared page list/// - comparison values for sorting link list eg, depth
        /*if(!sharedPageListComparisonBuffID_)
        glGenBuffers(1, &sharedPageListComparisonBuffID_);
        glBindBuffer(GL_TEXTURE_BUFFER, sharedPageListComparisonBuffID_);
        glBufferData(GL_TEXTURE_BUFFER, bufferSize*ABUFFER_DATA_TYPE_SIZE, NULL, GL_STATIC_DRAW);
        glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
        glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &sharedPageListComparisonAddress_);*/

		///Shared link pointer list///
		if(!sharedLinkListBuffID_) {
			glGenBuffers(1, &sharedLinkListBuffID_);
		    glBindBuffer(GL_TEXTURE_BUFFER, sharedLinkListBuffID_);
		    glBufferData(GL_TEXTURE_BUFFER, (bufferSize)*sizeof(glm::uint), NULL, GL_STATIC_DRAW);
		    glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
            glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &sharedLinkListAddress_); 
        }

        //LogInfo("d_sharedLinkList (Links) -> VBO "<< sharedLinkListBuffID_)
	}
    	
}

void Inviwo_ABufferGL4_LinkedList::initABuffer_LinkedList(){
        
	//Init occlusion query
	if(!totalFragmentQuery_)
		glGenQueries(1, &totalFragmentQuery_);
    
    size_t texIncr=0;
    std::vector<GLint> unitNums;
    const ImageGL* imageGL = 0;
    const LayerGL* layer = 0;
    TextureUnit* texUnit=0;

	if( pABufferUseTextures_ ) {
		///Page idx storage///		
        imageGL = abufferPageIdxImgTexture_->getRepresentation<ImageGL>();
        layer = imageGL->getColorLayerGL(0);
        texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);
        
        layer->bindTexture(texUnit->getEnum());		
		// Set filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
		glBindImageTextureEXT(texUnit->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,  GL_READ_WRITE, GL_R32UI);
		LGL_ERROR;
                
		///per-pixel page counter///		
        imageGL = abufferFragCountImgTexture_->getRepresentation<ImageGL>();
        layer = imageGL->getColorLayerGL(0);
        texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);

        layer->bindTexture(texUnit->getEnum());
		// Set filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindImageTextureEXT(texUnit->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,  GL_READ_WRITE, GL_R32UI);
        LGL_ERROR;

		///Semaphore///
        imageGL = semaphoreImgTexture_->getRepresentation<ImageGL>();
        layer = imageGL->getColorLayerGL(0);
        TextureUnit* texUnit = new TextureUnit();
        texUnits_.push_back(texUnit);

        layer->bindTexture(texUnit->getEnum());		
		// Set filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindImageTextureEXT(texUnit->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,  GL_READ_WRITE, GL_R32UI);
        LGL_ERROR;
		
	}else{

        LogWarn("Shared buffer not tested")

		///Page idx storage///
		if(!abufferPageIdxBuffID_)
			glGenBuffers(1, &abufferPageIdxBuffID_);
		glBindBuffer(GL_TEXTURE_BUFFER, abufferPageIdxBuffID_);
		glBufferData(GL_TEXTURE_BUFFER, dim_.x*dim_.y*sizeof(glm::uint32), NULL, GL_STATIC_DRAW);
		glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &abufferPageIdxAddress_);
		LGL_ERROR;

		///per-pixel page counter///
		if(!abufferFragCountBuffID_)
			glGenBuffers(1, &abufferFragCountBuffID_);
		glBindBuffer(GL_TEXTURE_BUFFER, abufferFragCountBuffID_);
		glBufferData(GL_TEXTURE_BUFFER, dim_.x*dim_.y*sizeof(glm::uint32), NULL, GL_STATIC_DRAW);
		glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &abufferFragCountAddress_);
		LGL_ERROR;

		///Semaphore///
		if(!semaphoreBuffID_)
			glGenBuffers(1, &semaphoreBuffID_);
		glBindBuffer(GL_TEXTURE_BUFFER, semaphoreBuffID_);
		glBufferData(GL_TEXTURE_BUFFER, dim_.x*dim_.y*sizeof(glm::uint32), NULL, GL_STATIC_DRAW);
		glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &semaphoreAddress_);
		LGL_ERROR;
	}
		
	initSharedPool_LinkedList();

	///Shared page counter///
	if(!curSharedPageBuffID_)
		glGenBuffers(1, &curSharedPageBuffID_);
	glBindBuffer(GL_ARRAY_BUFFER_ARB, curSharedPageBuffID_);
	glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(GLuint), NULL, GL_STATIC_DRAW);
	glMakeBufferResidentNV(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE);
	glGetBufferParameterui64vNV(GL_ARRAY_BUFFER_ARB, GL_BUFFER_GPU_ADDRESS_NV, &curSharedPageAddress_); 
    //LogInfo("d_curSharedPage (Counter) -> VBO "<< curSharedPageBuffID_)


    
    if (globalAtomicCounterBuffer_) {

        //bind atomic layer  (header)

        size_t headerCounterSize_ = 4;
        // http://www.lighthouse3d.com/tutorials/opengl-short-tutorials/opengl-atomic-counters/
        if(!globalAtomicsBufferId_)
            glGenBuffers(1, &globalAtomicsBufferId_);
        // bind the buffer and define its initial storage capacity
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, globalAtomicsBufferId_);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, headerCounterSize_*sizeof(GLuint), globalAtomicCounterBuffer_, GL_DYNAMIC_DRAW);
        // unbind the buffer
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }    
        
	    
	//char buff[256];
	//sprintf(buff, "[ABuffer Linked Lists] Memory usage: %.2fMB", (sharedPoolSize_*4*sizeof(float)/1024)/1024.0f);
}

void Inviwo_ABufferGL4_LinkedList::addUniforms(Shader* shader) {
   
	if( pABufferUseTextures_ ) {		
        shader->setUniform("abufferPageIdxImg", texUnits_[0]->getUnitNumber());
        shader->setUniform("abufferFragCountImg", texUnits_[1]->getUnitNumber());
        shader->setUniform("semaphoreImg",texUnits_[2]->getUnitNumber());   
	} else {
		glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_abufferPageIdx"), abufferPageIdxAddress_);
		glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_abufferFragCount"), abufferFragCountAddress_);
		glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_semaphore"), semaphoreAddress_);
        LogWarn("Shared buffer not tested")
	}

	if( pSharedPoolUseTextures_ ) { 
        shader->setUniform("shared_RGBA_DataPageListImg", texUnits_[3]->getUnitNumber());
        shader->setUniform("shared_EXT_DataPageListImg", texUnits_[4]->getUnitNumber());
        shader->setUniform("sharedLinkListImg", texUnits_[5]->getUnitNumber());
	} else {
		glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_shared_RGBA_DataPageList"), shared_RGBA_DataListAddress_);
        glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_shared_Ext_DataPageList"), shared_Ext_DataListAddress_);
		glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_sharedLinkList"), sharedLinkListAddress_);        
	}

	glProgramUniformui64NV(shader->getID(), glGetUniformLocation(shader->getID(), "d_curSharedPage"), curSharedPageAddress_);
	glProgramUniform1iEXT(shader->getID(), glGetUniformLocation(shader->getID(), "abufferSharedPoolSize"), static_cast<GLint>(sharedPoolSize_));
    //glProgramUniform1iEXT(shader->getID(), glGetUniformLocation(shader->getID(), "testAtomic"), globalAtomicsBufferId_);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, globalAtomicsBufferId_);

    LGL_ERROR;

    Inviwo_ABufferGL4::addUniforms(shader);

    LGL_ERROR;
}

bool Inviwo_ABufferGL4_LinkedList::display_LinkedList_ManageSharedPool() {
	//Resize shared pool dynamically
	lastFrameNumFrags_=0;
	if(queryRequested_){
		glGetQueryObjectuiv(totalFragmentQuery_, GL_QUERY_RESULT, &lastFrameNumFrags_);
	
		//A fragments is not discarded each time a page fails to be allocated
		if(lastFrameNumFrags_>0){
			sharedPoolSize_=sharedPoolSize_+(lastFrameNumFrags_/ABUFFER_PAGE_SIZE+1)*ABUFFER_PAGE_SIZE*2;
			initSharedPool_LinkedList();
			LogInfo("Shared buffer size increased");

			char buff[256];
			sprintf(buff, "[ABuffer Linked Lists] Memory usage: %.2fMB", (sharedPoolSize_*4*sizeof(float)/1024)/1024.0f);
			LogInfo("Shared buffer size increased" << buff);
			
		}

		queryRequested_=false;
	}
    LGL_ERROR;
	//checkGLError ("display_LinkedList_ManageSharedPool");
	if(lastFrameNumFrags_)
		return true;
    return false;
}


} // namespace