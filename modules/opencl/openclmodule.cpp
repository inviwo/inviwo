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

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/bufferclconverter.h>
#include <modules/opencl/buffer/bufferclglconverter.h>
#include <modules/opencl/buffer/elementbufferclglconverter.h>
#include <modules/opencl/image/layerclconverter.h>
#include <modules/opencl/image/layerclglconverter.h>
#include <modules/opencl/openclmodule.h>
#include <modules/opencl/openclexception.h>
#include <modules/opencl/openclcapabilities.h>
#include <modules/opencl/settings/openclsettings.h>
#include <modules/opencl/volume/volumeclconverter.h>
#include <modules/opencl/volume/volumeclglconverter.h>
#include <modules/opencl/kernelmanager.h>

#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

OpenCLModule::OpenCLModule(InviwoApplication* app) : InviwoModule(app, "OpenCL") {
    try {
        OpenCL::init();
        OpenCL::getPtr()->addCommonIncludeDirectory(getPath(ModulePath::CL));
        KernelManager::init();

        auto cap = util::make_unique<OpenCLCapabilities>();
        auto setting = util::make_unique<OpenCLSettings>(cap.get());
        registerCapabilities(std::move(cap));
        registerSettings(std::move(setting));
    } catch (OpenCLException& e) {
        throw ModuleInitException(e.getMessage(), e.getContext());
    }

    auto sharing = OpenCL::getPtr()->isOpenGLSharingEnabled();

    // Buffer CL
    registerRepresentationConverter(util::make_unique<BufferRAM2CLConverter>());
    registerRepresentationConverter(util::make_unique<BufferCL2RAMConverter>());
    
    if (sharing) {
        // Buffer CLGL
        registerRepresentationConverter(util::make_unique<BufferGL2CLGLConverter>());
        registerRepresentationConverter(util::make_unique<BufferCLGL2RAMConverter>());
        registerRepresentationConverter(util::make_unique<BufferCLGL2GLConverter>());
        registerRepresentationConverter(util::make_unique<BufferCLGL2CLConverter>());
        // ElementBuffer CLGL
        registerRepresentationConverter(util::make_unique<ElementBufferCLGL2GLConverter>());
        registerRepresentationConverter(util::make_unique<ElementBufferGL2CLGLConverter>());
    }
    // LayerCL
    registerRepresentationConverter(util::make_unique<LayerRAM2CLConverter>());
    registerRepresentationConverter(util::make_unique<LayerCL2RAMConverter>());
    
    if (sharing) {
        // LayerCLGL
        registerRepresentationConverter(util::make_unique<LayerGL2CLGLConverter>());
        registerRepresentationConverter(util::make_unique<LayerCLGL2RAMConverter>());
        registerRepresentationConverter(util::make_unique<LayerCLGL2CLConverter>());
        registerRepresentationConverter(util::make_unique<LayerCLGL2GLConverter>());
    }

    // VolumeCL
    registerRepresentationConverter(util::make_unique<VolumeRAM2CLConverter>());
    registerRepresentationConverter(util::make_unique<VolumeCL2RAMConverter>());
    
    if (sharing) {
        // VolumeCLGL
        registerRepresentationConverter(util::make_unique<VolumeGL2CLGLConverter>());
        registerRepresentationConverter(util::make_unique<VolumeCLGL2RAMConverter>());
        registerRepresentationConverter(util::make_unique<VolumeCLGL2CLConverter>());
        registerRepresentationConverter(util::make_unique<VolumeCLGL2GLConverter>());
    }
}

OpenCLModule::~OpenCLModule() {
    OpenCL::deleteInstance();
    KernelManager::deleteInstance();
}

} // namespace
