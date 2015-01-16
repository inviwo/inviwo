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

#include <inviwo/core/common/inviwocommondefines.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/opencl/image/layerclresizer.h>

namespace inviwo {


LayerCLResizer::LayerCLResizer() {
    try {
        cl::Program program = OpenCL::buildProgram(InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES)+"opencl/cl/img_resize.cl");
        resizeKernel_ = cl::Kernel(program, "resizeLayer");
    } catch (cl::Error&) {
    }
}

void LayerCLResizer::resize(const cl::Image& src, const cl::Image& dst, const uvec2& resizeToDimension)
{
    static LayerCLResizer instance;

    try {
        instance.getResizeKernel()->setArg(0, src);
        instance.getResizeKernel()->setArg(1, dst);
        cl::Event event;
        OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(instance.resizeKernel_, cl::NullRange, cl::NDRange(resizeToDimension[0],
                resizeToDimension[1]),
                cl::NullRange, NULL, &event);
        event.wait();
#if IVW_PROFILING
        LogInfoCustom("LayerCLResizer", "Image resizing from (" << src.getImageInfo<CL_IMAGE_WIDTH>() << ", " << src.getImageInfo<CL_IMAGE_HEIGHT>()
                      << ") to (" << resizeToDimension.x << ", " << resizeToDimension.y << ") in " << event.getElapsedTime() << " ms");
#endif
    } catch (cl::Error& err) {
        LogErrorCustom("LayerCLResizer", getCLErrorString(err));
    }
}
} // namespace
