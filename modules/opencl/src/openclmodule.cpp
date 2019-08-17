/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

        auto cap = std::make_unique<OpenCLCapabilities>();
        auto setting = std::make_unique<OpenCLSettings>(cap.get());
        registerCapabilities(std::move(cap));
        registerSettings(std::move(setting));
    } catch (OpenCLException& e) {
        throw ModuleInitException(e.getMessage(), e.getContext());
    }

    auto sharing = OpenCL::getPtr()->isOpenGLSharingEnabled();

    // Buffer CL
    registerRepresentationConverter<BufferRepresentation>(
        std::make_unique<BufferRAM2CLConverter>());
    registerRepresentationConverter<BufferRepresentation>(
        std::make_unique<BufferCL2RAMConverter>());

    if (sharing) {
        // Buffer CLGL
        registerRepresentationConverter<BufferRepresentation>(
            std::make_unique<BufferGL2CLGLConverter>());
        registerRepresentationConverter<BufferRepresentation>(
            std::make_unique<BufferCLGL2RAMConverter>());
        registerRepresentationConverter<BufferRepresentation>(
            std::make_unique<BufferCLGL2GLConverter>());
        registerRepresentationConverter<BufferRepresentation>(
            std::make_unique<BufferCLGL2CLConverter>());
    }
    // LayerCL
    registerRepresentationConverter<LayerRepresentation>(std::make_unique<LayerRAM2CLConverter>());
    registerRepresentationConverter<LayerRepresentation>(std::make_unique<LayerCL2RAMConverter>());

    if (sharing) {
        // LayerCLGL
        registerRepresentationConverter<LayerRepresentation>(
            std::make_unique<LayerGL2CLGLConverter>());
        registerRepresentationConverter<LayerRepresentation>(
            std::make_unique<LayerCLGL2RAMConverter>());
        registerRepresentationConverter<LayerRepresentation>(
            std::make_unique<LayerCLGL2CLConverter>());
        registerRepresentationConverter<LayerRepresentation>(
            std::make_unique<LayerCLGL2GLConverter>());
    }

    // VolumeCL
    registerRepresentationConverter<VolumeRepresentation>(
        std::make_unique<VolumeRAM2CLConverter>());
    registerRepresentationConverter<VolumeRepresentation>(
        std::make_unique<VolumeCL2RAMConverter>());

    if (sharing) {
        // VolumeCLGL
        registerRepresentationConverter<VolumeRepresentation>(
            std::make_unique<VolumeGL2CLGLConverter>());
        registerRepresentationConverter<VolumeRepresentation>(
            std::make_unique<VolumeCLGL2RAMConverter>());
        registerRepresentationConverter<VolumeRepresentation>(
            std::make_unique<VolumeCLGL2CLConverter>());
        registerRepresentationConverter<VolumeRepresentation>(
            std::make_unique<VolumeCLGL2GLConverter>());
    }
}

OpenCLModule::~OpenCLModule() {
    OpenCL::deleteInstance();
    KernelManager::deleteInstance();
}

}  // namespace inviwo
