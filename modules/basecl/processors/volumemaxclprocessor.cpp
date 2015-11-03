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

const ProcessorInfo VolumeMaxCLProcessor::processorInfo_{
    "org.inviwo.VolumeMaxCL",  // Class identifier
    "Volume Max",              // Display name
    "Volume Operation",        // Category
    CodeState::Experimental,   // Code state
    Tags::CL,                  // Tags
};
const ProcessorInfo VolumeMaxCLProcessor::getProcessorInfo() const {
    return processorInfo_;
}

VolumeMaxCLProcessor::VolumeMaxCLProcessor()
    : Processor()
    , ProcessorKernelOwner(this)
    , inport_("volume")
    , outport_("volume")
    , volumeRegionSize_("region", "Region size", 8, 1, 100)
    , workGroupSize_("wgsize", "Work group size", ivec3(8), ivec3(0), ivec3(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , supportsVolumeWrite_(false)
    , tmpVolume_(nullptr)
    , kernel_(nullptr) {
    addPort(inport_);
    addPort(outport_);

    addProperty(volumeRegionSize_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);

    buildKernel();
}

void VolumeMaxCLProcessor::process() {
    if (!kernel_) return;
    auto volume = inport_.getData();

    const size3_t dim{volume->getDimensions()};
    const size3_t outDim{glm::ceil(vec3(dim) / static_cast<float>(volumeRegionSize_.get()))};
    // const DataFormatBase* volFormat = inport_.getData()->getDataFormat(); // Not used

    if (!volumeOut_ || volumeOut_->getDimensions() != outDim) {
        volumeOut_ = std::make_shared<Volume>(outDim, DataUInt8::get());
        // volumeOut_ = std::unique_ptr<Volume>( new Volume(outDim, DataUInt32::get()) );
        // volumeOut_ = std::unique_ptr<Volume>( new Volume(outDim, DataFloat32::get()) );
        // Use same transformation to make sure that they are render at the same location
        volumeOut_->setModelMatrix(volume->getModelMatrix());
        volumeOut_->setWorldMatrix(volume->getWorldMatrix());
        outport_.setData(volumeOut_);
    }

    size3_t localWorkGroupSize(workGroupSize_.get());
    size3_t globalWorkGroupSize(getGlobalWorkGroupSize(outDim.x, localWorkGroupSize.x),
                                getGlobalWorkGroupSize(outDim.y, localWorkGroupSize.y),
                                getGlobalWorkGroupSize(outDim.z, localWorkGroupSize.z));

    if (useGLSharing_.get()) {
        SyncCLGL glSync;
        const VolumeCLGL* volumeCL = volume->getRepresentation<VolumeCLGL>();
        VolumeCLGL* volumeOutCL = volumeOut_->getEditableRepresentation<VolumeCLGL>();

        glSync.addToAquireGLObjectList(volumeCL);
        glSync.addToAquireGLObjectList(volumeOutCL);
        glSync.aquireAllObjects();

        executeVolumeOperation(volume.get(), volumeCL, volumeOutCL, outDim, globalWorkGroupSize,
                               localWorkGroupSize);
    } else {
        const VolumeCL* volumeCL = volume->getRepresentation<VolumeCL>();
        VolumeCL* volumeOutCL = volumeOut_->getEditableRepresentation<VolumeCL>();
        executeVolumeOperation(volume.get(), volumeCL, volumeOutCL, outDim, globalWorkGroupSize,
                               localWorkGroupSize);
    }
}

void VolumeMaxCLProcessor::executeVolumeOperation(const Volume* volume,
                                                  const VolumeCLBase* volumeCL,
                                                  VolumeCLBase* volumeOutCL, const size3_t& outDim,
                                                  const size3_t& globalWorkGroupSize,
                                                  const size3_t& localWorkgroupSize) {
    cl::Event events[2];
    try {
        BufferCL* tmpVolumeCL;
        int argIndex = 0;
        kernel_->setArg(argIndex++, *volumeCL);
        kernel_->setArg(argIndex++,
                        *(volumeCL->getVolumeStruct(volume)
                              .getRepresentation<BufferCL>()));  // Scaling for 12-bit data
        if (supportsVolumeWrite_) {
            kernel_->setArg(argIndex++, *volumeOutCL);
        } else {
            size_t outDimFlattened = outDim.x * outDim.y * outDim.z;
            if (tmpVolume_.get() == nullptr || tmpVolume_->getSize() != outDimFlattened) {
                tmpVolume_ = std::unique_ptr< Buffer<unsigned char> >(new Buffer<unsigned char>(outDimFlattened));
            }
            tmpVolumeCL = tmpVolume_->getEditableRepresentation<BufferCL>();
            kernel_->setArg(argIndex++, *tmpVolumeCL);
        }
        kernel_->setArg(argIndex++, ivec4(outDim, 0));
        kernel_->setArg(argIndex++, ivec4(volumeRegionSize_.get()));

        OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(
            *kernel_, cl::NullRange, globalWorkGroupSize, localWorkgroupSize, nullptr, &events[0]);

        if (!supportsVolumeWrite_) {
            std::vector<cl::Event> waitFor(1, events[0]);
            OpenCL::getPtr()->getQueue().enqueueCopyBufferToImage(
                tmpVolumeCL->get(), volumeOutCL->getEditable(), 0, size3_t(0), size3_t(outDim),
                &waitFor, &events[1]);
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
            LogInfo("Exec time (computation, copy): "
                    << events[0].getElapsedTime() << " + " << events[1].getElapsedTime() << " = "
                    << events[0].getElapsedTime() + events[1].getElapsedTime() << " ms");
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

}  // inviwo namespace

