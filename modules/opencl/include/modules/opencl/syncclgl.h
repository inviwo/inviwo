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

#ifndef IVW_SYNC_CL_GL_H
#define IVW_SYNC_CL_GL_H

#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/bufferclgl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/image/layerclgl.h>
#include <modules/opencl/volume/volumeclgl.h>

#ifdef CL_VERSION_1_1
// cl_GLsync was introduced in ver 1.1
#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif
// Define function clCreateEventFromGLsync (vendor extension)
typedef cl_event (*pfnclCreateEventFromSyncKHR)(cl_context context, cl_GLsync sync,
                                                cl_int* errcode_ret);
#endif  // CL_VERSION_1_1

namespace inviwo {
/** \class SyncCLGL
 * Helper for synchronizing OpenGL and OpenCL.
 * Calls glFinish(), or glFenceSync if supported, upon construction.
 * Releases all objects and calls clFinish,
 * or glWaitSync if supported, on the supplied queue upon destruction.
 */
class IVW_MODULE_OPENCL_API SyncCLGL {
public:
    /**
     * \brief Start synchronization of shared OpenGL and OpenCL objects
     *
     * Calls glFinish(), or glFenceSync if supported
     *
     */
    SyncCLGL(const cl::Context& context = OpenCL::getPtr()->getContext(),
             const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue());
    ~SyncCLGL();

    /**
     * Added object will be acquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually acquire the added object.
     * @param object
     */
    void addToAquireGLObjectList(const BufferCLGL* object);
    /**
     * Added object will be acquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually acquire the added object.
     * @param object
     */
    void addToAquireGLObjectList(const LayerCLGL* object);
    /**
     * Currently only adds the color layer to the list of objects that
     * will be acquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually acquire the added object.
     * @param object
     */
    void addToAquireGLObjectList(const ImageCLGL* object);
    /**
     * Added object will be acquired when calling aquireAllObject.
     * The added object will be released upon destruction of this SyncCLGL object.
     * @note aquireAllObjects need to be called to actually acquire the added object.
     * @param object
     */
    void addToAquireGLObjectList(const VolumeCLGL* object);

    /**
     * Call this function after adding objects to synchronize using addToAquireGLObjectList
     * Calls enqueueAcquireGLObjects on all previously added objects.
     * Will wait for provided events.
     * @note While you manually need to call aquireAllObjects, you do not need to call
     * releaseAllGLObjects as this will be done upon destruction of this object.
     */
    void aquireAllObjects(const std::vector<cl::Event>* waitForEvents = nullptr,
                          cl::Event* event = nullptr) const;

    /**
     * Release all added objects. Done automatically at destruction.
     * Will wait for provided events.
     * @note You do not need to call releaseAllGLObjects as this
     * will be done upon destruction of this object.
     */
    void releaseAllGLObjects(const std::vector<cl::Event>* waitForEvents = nullptr,
                             cl::Event* event = nullptr);

protected:
    std::vector<cl::Memory> syncedObjects_;
#if defined(CL_VERSION_1_1)
#include <warn/push>
#include <warn/ignore/ignored-attributes>
    // Faster synchronization might be supported when CL version >= 1.1

    // Store clCreateEventFromGLsync function per context
    // so that we only need to get them once
    static std::map<cl_context, pfnclCreateEventFromSyncKHR> syncFunctionMap_;
    GLsync glFenceSync_;  // OpenGL sync point - created in constructor
#include <warn/pop>
#endif
    const cl::Context& context_;
    const cl::CommandQueue& queue_;
};

}  // namespace inviwo

#endif  // IVW_SYNC_CL_GL_H