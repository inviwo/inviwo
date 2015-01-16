/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/util/settings/systemsettings.h>

#include <modules/opencl/settings/openclsettings.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelmanager.h>

namespace inviwo {

OpenCLSettings::OpenCLSettings(std::string id) :
    Settings(id)
    , openCLDeviceProperty_("openCLDevice","Default device")
    , enableOpenGLSharing_("glsharing", "Enable OpenGL sharing", true)
{}

OpenCLSettings::~OpenCLSettings() {
}

void OpenCLSettings::initialize() {
    std::vector<cl::Device> devices = OpenCL::getAllDevices();
    int defaultSelected = 0; 
    cl::Device bestDevice; cl::Platform platform;
    OpenCL::getBestGPUDeviceOnSystem(bestDevice, platform);
    for (size_t i = 0; i < devices.size(); ++i) {
        std::string name = devices[i].getInfo<CL_DEVICE_NAME>();
        if (bestDevice.getInfo<CL_DEVICE_NAME>() == name) {
            defaultSelected = static_cast<int>(i);
        }        
        openCLDeviceProperty_.addOption(name, name, static_cast<int>(i));
    }
    openCLDeviceProperty_.setSelectedIndex(defaultSelected);
    openCLDeviceProperty_.setCurrentStateAsDefault();
    addProperty(openCLDeviceProperty_);
    addProperty(enableOpenGLSharing_);
    openCLDeviceProperty_.onChange(this, &OpenCLSettings::changeDevice);
    enableOpenGLSharing_.onChange(this, &OpenCLSettings::changeDevice);
}

void OpenCLSettings::deinitialize()  {}

void OpenCLSettings::changeDevice() {
    // TODO: Close network before changing device. Load it again after changing
    //InviwoApplication::getPtr()->getProcessorNetwork()->
    KernelManager::getPtr()->clear();
    std::vector<cl::Device> devices = OpenCL::getAllDevices();

    // Assuming that the number of devices did not change since initialize()
    // was called
    if (openCLDeviceProperty_.get() < static_cast<int>(devices.size())) {
        if (!devices[openCLDeviceProperty_.get()].getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_gl_sharing")) {
            enableOpenGLSharing_.set(false);
            enableOpenGLSharing_.setReadOnly(true);
        } else {
            enableOpenGLSharing_.setReadOnly(false);
        }
        OpenCL::getPtr()->setDevice(devices[openCLDeviceProperty_.get()], enableOpenGLSharing_.get());
    }
}

} // namespace
