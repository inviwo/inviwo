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

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/inviwoapplication.h>
#include <inviwo/core/util/logdistributor.h>

#include <modules/opencl/openclmodule.h>
#include <modules/opencl/inviwoopencl.h>

#include <modules/opencl/imagecl.h>
#include <inviwo/core/datastructures/image.h>
#include <inviwo/core/datastructures/imageram.h>
#include <cstdio>


int test_buffer()
{
    int error(0);
    int test[4] = {0, 1, 2, 3};

    try {
        cl::Buffer buffer(CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(test), test);
        int result[4];
        inviwo::OpenCL::getInstance()->getQueue().enqueueReadBuffer(buffer, true, 0, sizeof(test), result);

        for (int i = 0; i < 4; ++i) {
            error |= test[i] != result[i];
        }
    } catch (cl::Error) {
        error +=1;
    }

    return error;
}

int test_image() {
    int error(0);
    // Create image representations and module
    inviwo::InviwoApplication app("Test", "");
    app.initialize();
    //uint8_t imageData[16] = {1, 1, 1, 1,
    //    0, 0, 0, 0,
    //    0, 0, 0, 0,
    //    0, 0, 0, 0};
    //cl::Image2D img(*inviwo::OpenCL::getInstance()->getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, CL_FLOAT, 4, 4, imageData);
    uvec2 imageSize(512, 503);
    std::vector<uint8_t> imageData(imageSize.x*imageSize.y);
    inviwo::Image image(imageSize);
    inviwo::ImageRAM* ram = image.getRepresentation<inviwo::ImageRAM>();

    try {
        inviwo::ImageCL* imageCL = image.getRepresentation<inviwo::ImageCL>();
        inviwo::OpenCL::getInstance()->getQueue().enqueueWriteImage(*(imageCL->getImage()), true, glm::svec3(0), glm::svec3(imageSize, 1), 0, 0,
                &imageData[0]);
        uvec2 resizeTo(212, 103);
        imageCL->resize(resizeTo);
        std::vector<uint8_t> resizedImageData(resizeTo.x*resizeTo.y);
        inviwo::OpenCL::getInstance()->getQueue().enqueueReadImage(*(imageCL->getImage()), true, glm::svec3(0), glm::svec3(resizeTo, 1), 0, 0,
                &resizedImageData[0]);
    } catch (cl::Error) {
        error +=1;
    }

    return error;
}

int main()
{
    // Enable output to console
    inviwo::LogCentral::instance()->registerLogger(new inviwo::ConsoleLogger());
    int error(0);
    error += test_buffer();
    error += test_image();
    return error;
}

