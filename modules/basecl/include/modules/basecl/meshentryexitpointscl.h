/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_MESHENTRYEXITPOINTSCL_H
#define IVW_MESHENTRYEXITPOINTSCL_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/bufferclbase.h>
#include <modules/opencl/image/layerclbase.h>
#include <modules/opencl/kernelowner.h>

namespace inviwo {
/**
 * Computes entry and exit points (in texture coordinates) given a camera and viewport.
 */
class IVW_MODULE_BASECL_API MeshEntryExitPointsCL : public KernelOwner {
public:
    MeshEntryExitPointsCL(const glm::size2_t& workGroupSize = size2_t(16));
    virtual ~MeshEntryExitPointsCL() {}

    /**
     * \brief Computes entry exit points in texture coordinates
     */
    bool computeEntryExitPoints(const Mesh* mesh, const mat4& worldToView, const mat4& viewToClip,
                                Layer* entryPoints, Layer* exitPoints, bool useGLSharing,
                                const VECTOR_CLASS<cl::Event>* waitForEvents = nullptr,
                                cl::Event* event = nullptr);

    void computeEntryExitPoints(const mat4& NDCToTextureMat, const mat4& worldToTextureMat,
                                const BufferCLBase* vertices, const BufferCLBase* indices,
                                int nIndices, const LayerCLBase* entryPointsCL,
                                const LayerCLBase* exitPointsCL, const uvec2& outportDim,
                                const VECTOR_CLASS<cl::Event>* waitForEvents = nullptr,
                                cl::Event* event = nullptr);

    size2_t getWorkGroupSize() const { return workGroupSize_; }
    void setWorkGroupSize(size2_t val) { workGroupSize_ = val; }

private:
    glm::size2_t workGroupSize_;

    cl::Kernel* kernel_;
};

}  // namespace inviwo

#endif  // IVW_MESHENTRYEXITPOINTSCL_H
