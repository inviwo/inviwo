/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/bufferclconverter.h>
#include <modules/opencl/buffer/bufferclglconverter.h>
#include <modules/opencl/buffer/elementbufferclglconverter.h>
#include <modules/opencl/image/layerclconverter.h>
#include <modules/opencl/image/layerclglconverter.h>
#include <modules/opencl/openclmodule.h>
#include <modules/opencl/openclcapabilities.h>
#include <modules/opencl/settings/openclsettings.h>
#include <modules/opencl/volume/volumeclconverter.h>
#include <modules/opencl/volume/volumeclglconverter.h>
#include <modules/opencl/kernelmanager.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {

OpenCLModule::OpenCLModule() : InviwoModule(), btnOpenCLInfo_("printOpenCLInfo", "Print OpenCL Info") {
    setIdentifier("OpenCL");

    // Buffer CL
    registerRepresentationConverter(new BufferRAM2CLConverter());
    registerRepresentationConverter(new BufferCL2RAMConverter());
    registerRepresentationConverter(new BufferGL2CLConverter());
    // Buffer CLGL
    registerRepresentationConverter(new BufferRAM2CLGLConverter());
    registerRepresentationConverter(new BufferGL2CLGLConverter());
    registerRepresentationConverter(new BufferCLGL2RAMConverter());
    registerRepresentationConverter(new BufferCLGL2GLConverter());
    registerRepresentationConverter(new BufferCL2GLConverter());
    registerRepresentationConverter(new BufferCLGL2CLConverter());
    registerRepresentationConverter(new BufferCL2CLGLConverter());
    // ElementBuffer CLGL
    registerRepresentationConverter(new ElementBufferRAM2CLGLConverter());
    registerRepresentationConverter(new ElementBufferCLGL2GLConverter());
    registerRepresentationConverter(new ElementBufferGL2CLGLConverter());
    registerRepresentationConverter(new ElementBufferCL2CLGLConverter());
    // LayerCL
    registerRepresentationConverter(new LayerDisk2CLConverter());
    registerRepresentationConverter(new LayerRAM2CLConverter());
    registerRepresentationConverter(new LayerCL2RAMConverter());
    registerRepresentationConverter(new LayerGL2CLConverter());
    registerRepresentationConverter(new LayerCL2GLConverter());
    // LayerCLGL
    registerRepresentationConverter(new LayerDisk2CLGLConverter());
    registerRepresentationConverter(new LayerRAM2CLGLConverter());
    registerRepresentationConverter(new LayerGL2CLGLConverter());
    registerRepresentationConverter(new LayerCL2CLGLConverter());
    registerRepresentationConverter(new LayerCLGL2RAMConverter());
    registerRepresentationConverter(new LayerCLGL2CLConverter());
    registerRepresentationConverter(new LayerCLGL2GLConverter());
    registerRepresentationConverter(new LayerGL2CLGLConverter());
    // VolumeCL
    registerRepresentationConverter(new VolumeDisk2CLConverter());
    registerRepresentationConverter(new VolumeRAM2CLConverter());
    registerRepresentationConverter(new VolumeCL2RAMConverter());
    registerRepresentationConverter(new VolumeGL2CLConverter());
    registerRepresentationConverter(new VolumeCL2GLConverter());
    // VolumeCLGL
    registerRepresentationConverter(new VolumeDisk2CLGLConverter());
    registerRepresentationConverter(new VolumeRAM2CLGLConverter());
    registerRepresentationConverter(new VolumeGL2CLGLConverter());
    registerRepresentationConverter(new VolumeCL2CLGLConverter());
    registerRepresentationConverter(new VolumeCLGL2RAMConverter());
    registerRepresentationConverter(new VolumeCLGL2CLConverter());
    registerRepresentationConverter(new VolumeCLGL2GLConverter());
    registerRepresentationConverter(new VolumeGL2CLGLConverter());
    registerCapabilities(new OpenCLCapabilities());
    registerSettings(new OpenCLSettings());

}

void OpenCLModule::initialize() {
    OpenCL::init();

    OpenCL::getPtr()->addCommonIncludeDirectory(InviwoApplication::PATH_MODULES, "opencl/cl");
    
    KernelManager::init();
    InviwoModule::initialize();
}

void OpenCLModule::setupModuleSettings() {
    //New OpengCLSettings class can be created if required.
    SystemSettings* settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();

    if (settings) {
        OpenCLCapabilities* openclInfo = getTypeFromVector<OpenCLCapabilities>(getCapabilities());

        if (openclInfo) {
            btnOpenCLInfo_.onChange(openclInfo, &OpenCLCapabilities::printDetailedInfo);
            settings->addProperty(&btnOpenCLInfo_);
        }
    }

    InviwoModule::setupModuleSettings();
}

} // namespace
