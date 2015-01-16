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

#ifndef IVW_SYNC_CL_GL_H
#define IVW_SYNC_CL_GL_H

#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/bufferclgl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/image/layerclgl.h>
#include <modules/opencl/volume/volumeclgl.h>

#ifdef CL_VERSION_1_1
#ifdef __APPLE__
#include <OpenCL/cl_gl_ext.h>
#else
#include <CL/cl_gl_ext.h>
#endif
#endif

namespace inviwo {

class IVW_MODULE_OPENCL_API SyncCLGL {
public:
    SyncCLGL(const cl::Context& context= OpenCL::getPtr()->getContext(), const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue());
    ~SyncCLGL();

    /**
     * Added object will be enquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually enquire the added object.
     * @param const BufferCLGL * object
     */
    void addToAquireGLObjectList(const BufferCLGL* object);
    /**
     * Added object will be enquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually enquire the added object.
     * @param const LayerCLGL * object
     */
    void addToAquireGLObjectList(const LayerCLGL* object);
    /**
     * Currently only adds the color layer to the list of objects that
     * will be enquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually enquire the added object.
     * @param const ImageCLGL * object
     */
    void addToAquireGLObjectList(const ImageCLGL* object);
    /**
     * Added object will be enquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually enquire the added object.
     * @param const VolumeCLGL * object
     */
    void addToAquireGLObjectList(const VolumeCLGL* object);

    /**
     * Calls enqueueAcquireGLObjects on all previously added objects
     *
     * @note Do not call enqueueReleaseGLObjects on an added object as this
     * will be done upon destruction of this object.
     */
    void aquireAllObjects() const;

    std::vector<cl::Event>* getGLSyncEvent() { return syncEvents_; }

    cl::Event* getLastReleaseGLEvent() { return releaseEvent_; }

protected:
    /**
     * Release all added objects.
     */
    void releaseAllGLObjects() const;

    std::vector<cl::Memory> syncedObjects_;
#if defined(CL_VERSION_1_1)
    GLsync glFenceSync_;
    cl::Event glSync_;
#endif
    cl::Event* releaseEvent_;
    std::vector<cl::Event>* syncEvents_;
    const cl::Context& context_;
    const cl::CommandQueue& queue_;
};

}

#endif // IVW_SYNC_CL_GL_H