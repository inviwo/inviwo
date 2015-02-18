 /*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <modules/basecl/processors/volumemaxclprocessor.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/buffer/bufferclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeMaxCLProcessor, "org.inviwo.VolumeMaxCL");
ProcessorDisplayName(VolumeMaxCLProcessor,  "Volume Max");
ProcessorTags(VolumeMaxCLProcessor, Tags::CL);
ProcessorCategory(VolumeMaxCLProcessor, "Volume Operation");
ProcessorCodeState(VolumeMaxCLProcessor, CODE_STATE_EXPERIMENTAL);

VolumeMaxCLProcessor::VolumeMaxCLProcessor()
    : Processor(), ProcessorKernelOwner(this)
    , inport_("volume")
    , outport_("volume")
    , volumeRegionSize_("region", "Region size", 8, 1, 100)
    , workGroupSize_("wgsize", "Work group size", ivec3(8), ivec3(0), ivec3(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , supportsVolumeWrite_(false)
    , tmpVolume_(NULL)
    , kernel_(NULL)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(volumeRegionSize_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);
}

VolumeMaxCLProcessor::~VolumeMaxCLProcessor() {}

void VolumeMaxCLProcessor::initialize() {
    Processor::initialize();
    buildKernel();
}

void VolumeMaxCLProcessor::deinitialize() {
    Processor::deinitialize();
    delete tmpVolume_;
    tmpVolume_ = NULL;
}

void VolumeMaxCLProcessor::process() {
    if (kernel_ == NULL) {
        return;
    }
    const Volume* volume = inport_.getData();
    
    uvec3 dim = volume->getDimensions();
    uvec3 outDim = uvec3(glm::ceil(vec3(dim)/static_cast<float>(volumeRegionSize_.get())));
    //const DataFormatBase* volFormat = inport_.getData()->getDataFormat(); // Not used


    if (!outport_.hasData() || outport_.getData()->getDimensions() != outDim) {
        Volume* volumeOut = new Volume(outDim, DataUINT8::get());
        //volumeOut = new Volume(outDim, DataUINT32::get());
        //volumeOut = new Volume(outDim, DataFLOAT32::get());
        // Use same transformation to make sure that they are render at the same location
        volumeOut->setModelMatrix(volume->getModelMatrix());
        volumeOut->setWorldMatrix(volume->getWorldMatrix());
        outport_.setData(volumeOut);
    }
    Volume* volumeOut = outport_.getData();

    svec3 localWorkGroupSize(workGroupSize_.get());
    svec3 globalWorkGroupSize(getGlobalWorkGroupSize(outDim.x, localWorkGroupSize.x),
                              getGlobalWorkGroupSize(outDim.y, localWorkGroupSize.y),  
                              getGlobalWorkGroupSize(outDim.z, localWorkGroupSize.z));

    if (useGLSharing_.get()) {
        SyncCLGL glSync;
        const VolumeCLGL* volumeCL = volume->getRepresentation<VolumeCLGL>();
        VolumeCLGL* volumeOutCL = volumeOut->getEditableRepresentation<VolumeCLGL>();

        glSync.addToAquireGLObjectList(volumeCL);
        glSync.addToAquireGLObjectList(volumeOutCL);
        glSync.aquireAllObjects();
        
        executeVolumeOperation(volume, volumeCL, volumeOutCL, outDim, globalWorkGroupSize, localWorkGroupSize);
    } else {
        const VolumeCL* volumeCL = volume->getRepresentation<VolumeCL>();
        VolumeCL* volumeOutCL = volumeOut->getEditableRepresentation<VolumeCL>();
        executeVolumeOperation(volume, volumeCL, volumeOutCL, outDim, globalWorkGroupSize, localWorkGroupSize);
    }
    

}

void VolumeMaxCLProcessor::executeVolumeOperation(const Volume* volume, const VolumeCLBase* volumeCL, VolumeCLBase* volumeOutCL, const uvec3& outDim,
                                         const svec3& globalWorkGroupSize, const svec3& localWorkgroupSize) {
    cl::Event events[2];
    try {
        BufferCL* tmpVolumeCL;
        int argIndex = 0;
        kernel_->setArg(argIndex++, *volumeCL);
        kernel_->setArg(argIndex++, *(volumeCL->getVolumeStruct(volume).getRepresentation<BufferCL>())); // Scaling for 12-bit data
        if (supportsVolumeWrite_) {
            kernel_->setArg(argIndex++, *volumeOutCL);
        } else {
            size_t outDimFlattened = outDim.x*outDim.y*outDim.z;
            if (tmpVolume_ == NULL || tmpVolume_->getSize() != outDimFlattened) {
                delete tmpVolume_; tmpVolume_ = new Buffer_UINT8(outDimFlattened);
                //delete tmpVolume_; tmpVolume_ = new Buffer_UINT32(outDimFlattened);
                //delete tmpVolume_; tmpVolume_ = new Buffer_FLOAT32(outDimFlattened);
            }
            tmpVolumeCL = tmpVolume_->getEditableRepresentation<BufferCL>();
            kernel_->setArg(argIndex++, *tmpVolumeCL);
        }
        kernel_->setArg(argIndex++, ivec4(outDim, 0));
        kernel_->setArg(argIndex++, ivec4(volumeRegionSize_.get()));
               
        OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, globalWorkGroupSize , localWorkgroupSize, NULL, &events[0]);

        if (!supportsVolumeWrite_) {
            std::vector<cl::Event> waitFor(1, events[0]);
            OpenCL::getPtr()->getQueue().enqueueCopyBufferToImage(tmpVolumeCL->get(), volumeOutCL->getEditable(), 0, svec3(0), svec3(outDim), &waitFor, &events[1]);
        }
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }

#if IVW_PROFILING 
    try { 
        if (supportsVolumeWrite_) {
            events[0].wait();
            LogInfo("Exec time: " << events[0].getElapsedTime() << " ms");
        } else {
            // Measure both computation and copy (only need to wait for copy)
            events[1].wait();
            LogInfo("Exec time (computation, copy): " << events[0].getElapsedTime() << " + " << events[1].getElapsedTime() << " = " << events[0].getElapsedTime()+events[1].getElapsedTime() << " ms");
        }
    } catch (cl::Error& err) { 
        LogError(getCLErrorString(err)); 
    } 
#endif
}

void VolumeMaxCLProcessor::buildKernel() {
    std::stringstream defines;
    std::string extensions = OpenCL::getPtr()->getDevice().getInfo<CL_DEVICE_EXTENSIONS>();
    if (extensions.find("cl_khr_3d_image_writes") != std::string::npos) {
        supportsVolumeWrite_ = true;
        defines << " -D SUPPORTS_VOLUME_WRITE "; 
    } else {
        supportsVolumeWrite_ = false;
    }
    kernel_ = addKernel("volumemax.cl", "volumeMaxKernel", defines.str());

}



} // inviwo namespace
