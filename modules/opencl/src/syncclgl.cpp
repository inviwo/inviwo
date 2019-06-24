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

#include <modules/opencl/syncclgl.h>
#include <modules/opencl/openclexception.h>

namespace inviwo {

#if defined(CL_VERSION_1_1)
#include <warn/push>
#include <warn/ignore/ignored-attributes>
std::map<cl_context, pfnclCreateEventFromSyncKHR> SyncCLGL::syncFunctionMap_;
#include <warn/pop>
#endif

SyncCLGL::SyncCLGL(const cl::Context& context, const cl::CommandQueue& queue)
    :
#if defined(CL_VERSION_1_1)
    glFenceSync_(nullptr)
#endif
    , context_(context)
    , queue_(queue) {
#if defined(CL_VERSION_1_1)
    // Check if function clCreateEventFromGLsyncKHR has been fetched previously
    // and that glCreateSyncFromCLeventARB exist (non-existing on Mac).
    if (syncFunctionMap_.find(context()) == syncFunctionMap_.end() && glCreateSyncFromCLeventARB) {
        // Get clCreateEventFromGLsyncKHR function from platform since
        // it is a vendor extension and cannot be statically linked
#if defined(CL_VERSION_1_2)  // version >= 1.2
        // Function was renamed in version 1.2
        auto device = queue.getInfo<CL_QUEUE_DEVICE>();
        auto platform = device.getInfo<CL_DEVICE_PLATFORM>();
        syncFunctionMap_[context()] =
            (pfnclCreateEventFromSyncKHR)clGetExtensionFunctionAddressForPlatform(
                platform, "clCreateEventFromGLsyncKHR");
#else  // Version 1.1
       // Requires cl_khr_gl_sharing extension. Extension is supported since we are using sharing
        syncFunctionMap_[context()] = (pfnclCreateEventFromSyncKHR)clGetExtensionFunctionAddress(
            "clCreateEventFromGLsyncKHR");
#endif
    }
    pfnclCreateEventFromSyncKHR clCreateEventFromGLsync = syncFunctionMap_[context()];
    if (clCreateEventFromGLsync) {
        // Use more efficient synchronization
        // See section 9.9 in the OpenCL 1.1 spec for more information, also
        // https://www.cct.lsu.edu/~korobkin/tmp/SC10/tutorials/docs/M13/M13.pdf

        // Get sync point from OpenGL to be used when acquiring objects
        // Note that glFenceSync is supported since it exit in OpenGl 3.2 and higher
        glFenceSync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    } else {
        // clCreateEventFromGLsync not supported.
        // We need to use a slower synchronization
        glFinish();
    }
#else
    glFinish();
#endif
}

SyncCLGL::~SyncCLGL() {
    releaseAllGLObjects();
#if defined(CL_VERSION_1_1)
    if (glFenceSync_) glDeleteSync(glFenceSync_);
#endif
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

void SyncCLGL::aquireAllObjects(const std::vector<cl::Event>* waitForEvents /*= nullptr*/,
                                cl::Event* event /*= nullptr*/) const {
#if defined(CL_VERSION_1_1)
    // Use fast synchronization if we can
    if (glFenceSync_) {
        pfnclCreateEventFromSyncKHR clCreateEventFromGLsync = syncFunctionMap_[context_()];
        // Sync with OpenGL
        cl_int err;
        std::vector<cl::Event> waitForSyncAndEvents(
            1, clCreateEventFromGLsync(context_(), glFenceSync_, &err));
        if (err != CL_SUCCESS) {
            throw OpenCLException("Failed to create sync event");
        }
        if (waitForEvents) {
            // Add events to wait for
            waitForSyncAndEvents.reserve(waitForEvents->size() + 1);
            waitForSyncAndEvents.insert(std::end(waitForSyncAndEvents), std::begin(*waitForEvents),
                                        std::end(*waitForEvents));
        }
        queue_.enqueueAcquireGLObjects(&syncedObjects_, &waitForSyncAndEvents, event);
    } else {
        // Fast sync not supported, glFinish has been called in constructor instead
        queue_.enqueueAcquireGLObjects(&syncedObjects_, waitForEvents, event);
    }
#else
    queue_.enqueueAcquireGLObjects(&syncedObjects_, waitForEvents, event);
#endif
}

void SyncCLGL::releaseAllGLObjects(const std::vector<cl::Event>* waitForEvents, cl::Event* event) {
    if (!syncedObjects_.empty()) {
#if defined(CL_VERSION_1_1)
        if (glFenceSync_) {
            cl::Event releaseEvent;
            // Use supplied event if existing
            cl::Event* releaseEventPtr = event != nullptr ? event : &releaseEvent;
            queue_.enqueueReleaseGLObjects(&syncedObjects_, waitForEvents, releaseEventPtr);
            // Synchronize OpenCL and OpenGL
            GLsync clSync = glCreateSyncFromCLeventARB(context_(), (*releaseEventPtr)(), 0);
            // without stalling CPU-thread:
            glWaitSync(clSync, 0, GL_TIMEOUT_IGNORED);
        } else {
            queue_.enqueueReleaseGLObjects(&syncedObjects_, waitForEvents, event);
            queue_.finish();
        }
#else
        queue_.enqueueReleaseGLObjects(&syncedObjects_, waitForEvents, event);
        queue_.finish();
#endif
        syncedObjects_.clear();
    }
}

}  // namespace inviwo
