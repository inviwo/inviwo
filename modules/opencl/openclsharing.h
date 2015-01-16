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

#ifndef IVW_OPENCL_SHARING_H
#define IVW_OPENCL_SHARING_H


#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opengl/glwrap/texture.h>
#include <modules/opengl/glwrap/bufferobject.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/util/referencecounter.h>
#include <map>

namespace inviwo {
class OpenCLImageSharing;
class LayerCLGL;
class VolumeCLGL;

typedef std::pair<Texture*, OpenCLImageSharing*> TextureCLImageSharingPair;
typedef std::map<Texture*, OpenCLImageSharing*> CLTextureSharingMap;


class IVW_MODULE_OPENCL_API OpenCLImageSharing: public ReferenceCounter {
    friend class LayerCLGL;
    friend class VolumeCLGL;
public:
    OpenCLImageSharing(cl::Image* sharedMemory = NULL): ReferenceCounter(), sharedMemory_(sharedMemory) {}

    cl::Image* sharedMemory_;
private:
    static CLTextureSharingMap clImageSharingMap_;
};

class OpenCLBufferSharing;

typedef std::pair<BufferObject*, OpenCLBufferSharing*> BufferSharingPair;
typedef std::map<BufferObject*, OpenCLBufferSharing*> CLBufferSharingMap;

class IVW_MODULE_OPENCL_API OpenCLBufferSharing: public ReferenceCounter {
    friend class BufferCLGL;
public:
    OpenCLBufferSharing(cl::Buffer* sharedMemory = NULL): ReferenceCounter(), sharedMemory_(sharedMemory) {}

    cl::Buffer* sharedMemory_;
private:

    static CLBufferSharingMap clBufferSharingMap_;
};


}

#endif // IVW_OPENCL_SHARING_H