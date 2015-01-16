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

#include <modules/opencl/syncclgl.h>

/*
typedef cl_event (*PFNCLCREATEEVENTFROMGLSYNCKHR) (cl_context context, cl_GLsync sync, cl_int
*errcode_ret);
PFNCLCREATEEVENTFROMGLSYNCKHR clCreateEventFromGLsync =
(PFNCLCREATEEVENTFROMGLSYNCKHR)clGetExtensionFunctionAddress("clCreateEventFromGLsyncKHR");
 */

namespace inviwo {

SyncCLGL::SyncCLGL(const cl::Context& context, const cl::CommandQueue& queue)
    : releaseEvent_(NULL), syncEvents_(NULL), context_(context), queue_(queue) {
#if defined(CL_VERSION_1_1) && defined(GL_ARB_cl_event) && \
    defined(CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR)
    // FIXME: We can only do this if it is supported by the device, otherwise we get unresolved
    // symbol from clCreateEventFromGLsyncKHR
    // glFenceSync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    // cl_int err;
    // glSync_ = clCreateEventFromGLsyncKHR(context(), glFenceSync_, &err);
    // syncEvents_ = new std::vector<cl::Event>(1, glSync_);
    // if(err != CL_SUCCESS) {
    //    LogError("Failed to create sync event");
    //}
    // releaseEvent_ = new cl::Event();
    glFinish();
#else
    glFinish();
#endif
}

SyncCLGL::~SyncCLGL() {
    try {
        releaseAllGLObjects();
#if defined(CL_VERSION_1_1) && defined(GL_ARB_cl_event) && \
    defined(CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR)
        // GLsync clSync = glCreateSyncFromCLeventARB(context_(), (*releaseEvent_)(), 0);
        // glWaitSync(clSync, 0, GL_TIMEOUT_IGNORED);
        // glDeleteSync(glFenceSync_);
        // delete syncEvents_;
        // delete releaseEvent_;
        queue_.finish();
#else
        queue_.finish();
#endif
    }
    catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}

void SyncCLGL::addToAquireGLObjectList(const BufferCLGL* object) {
    syncedObjects_.push_back(object->get());
}

void SyncCLGL::addToAquireGLObjectList(const LayerCLGL* object) {
    syncedObjects_.push_back(object->get());
}

void SyncCLGL::addToAquireGLObjectList(const ImageCLGL* object) {
    addToAquireGLObjectList(object->getLayerCL());
}

void SyncCLGL::addToAquireGLObjectList(const VolumeCLGL* object) {
    syncedObjects_.push_back(object->get());
}

void SyncCLGL::aquireAllObjects() const {
    queue_.enqueueAcquireGLObjects(&syncedObjects_, syncEvents_);
}

void SyncCLGL::releaseAllGLObjects() const {
    if (!syncedObjects_.empty())
        queue_.enqueueReleaseGLObjects(&syncedObjects_, NULL, releaseEvent_);
}

}  // end namespace
