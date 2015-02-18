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

#ifndef IVW_IMAGE_CL_RESIZER_H
#define IVW_IMAGE_CL_RESIZER_H

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>

namespace inviwo {

/** \class LayerCLResizer
 *
 * Helper class that resizes a 2D layer.
 * @note It will compile the OpenCL kernel the first time resize is called (may take some time).
 * @see LayerCL
 */
class IVW_MODULE_OPENCL_API LayerCLResizer {

public:

    /**
     * Resize layer to given dimension.
     *
     * \param src (const cl::Image2D &) Layer to get data from
     * \param dst (const cl::Image2D &) Layer containing resized src layer. Note that this should same dimensions as resizeToDimension
     * \param resizeToDimension (const ivec2 &) Size to resize to
     * \return (void)
     */
    static void resize(const cl::Image& src, const cl::Image& dst, const uvec2& resizeToDimension);

private:
    LayerCLResizer();
    LayerCLResizer(LayerCLResizer const&) {};
    void operator=(LayerCLResizer const&) {};
    /**
     * Kernel that takes two layers as input. First layer acts as source and second as destination.
     *
     * \return (cl::Kernel*)
     */
    cl::Kernel* getResizeKernel() { return &resizeKernel_; }

    cl::Kernel resizeKernel_;


};


} // namespace

#endif // IVW_IMAGE_CL_RESIZER_H
