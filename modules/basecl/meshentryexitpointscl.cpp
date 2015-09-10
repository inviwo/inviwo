/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include "meshentryexitpointscl.h"
#include <modules/opencl/image/layercl.h>
#include <modules/opencl/image/layerclgl.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/buffer/elementbuffercl.h>
#include <modules/opencl/buffer/elementbufferclgl.h>
#include <modules/opencl/syncclgl.h>

namespace inviwo {

MeshEntryExitPointsCL::MeshEntryExitPointsCL(const glm::size2_t& workGroupSize /*= size2_t(16)*/)
    : workGroupSize_(workGroupSize), kernel_(nullptr) {
    kernel_ = addKernel("entryexitpoints.cl", "entryExitPointsKernel");
}

bool MeshEntryExitPointsCL::computeEntryExitPoints(
    const Mesh* mesh, const mat4& worldToView, const mat4& viewToClip, Layer* entryPoints,
    Layer* exitPoints, bool useGLSharing,
    const VECTOR_CLASS<cl::Event>* waitForEvents /*= nullptr*/, cl::Event* event /*= nullptr*/) {
    if (kernel_ == nullptr) {
        return false;
    }
    // the rendered plane is specified in camera coordinates
    // thus we must transform from camera to world to texture coordinates
    mat4 worldToTexMat = mesh->getCoordinateTransformer().getWorldToDataMatrix();
    uvec2 outportDim = entryPoints->getDimensions();
    mat4 NDCToTextureMat = worldToTexMat * glm::inverse(worldToView) * glm::inverse(viewToClip);

    int nIndices = static_cast<int>(mesh->getIndicies(0)->getSize());
    if (useGLSharing) {
        SyncCLGL glSync;
        LayerCLGL* entryCL = entryPoints->getEditableRepresentation<LayerCLGL>();
        LayerCLGL* exitCL = exitPoints->getEditableRepresentation<LayerCLGL>();
        const BufferCLGL* vertices = mesh->getAttributes(0)->getRepresentation<BufferCLGL>();
        const BufferCLGL* indices = mesh->getIndicies(0)->getRepresentation<ElementBufferCLGL>();
        glSync.addToAquireGLObjectList(entryCL);
        glSync.addToAquireGLObjectList(exitCL);
        glSync.addToAquireGLObjectList(vertices);
        glSync.addToAquireGLObjectList(indices);
        glSync.aquireAllObjects();

        computeEntryExitPoints(NDCToTextureMat, worldToTexMat, vertices, indices, nIndices, entryCL,
                               exitCL, outportDim, waitForEvents, event);
    } else {
        LayerCL* entryCL = entryPoints->getEditableRepresentation<LayerCL>();
        LayerCL* exitCL = exitPoints->getEditableRepresentation<LayerCL>();
        const BufferCL* vertices = mesh->getAttributes(0)->getRepresentation<BufferCL>();
        const BufferCL* indices = mesh->getIndicies(0)->getRepresentation<ElementBufferCL>();
        computeEntryExitPoints(NDCToTextureMat, worldToTexMat, vertices, indices, nIndices, entryCL,
                               exitCL, outportDim, waitForEvents, event);
    }

    return true;
}

void MeshEntryExitPointsCL::computeEntryExitPoints(
    const mat4& NDCToTextureMat, const mat4& worldToTextureMat, const BufferCLBase* vertices,
    const BufferCLBase* indices, int nIndices, const LayerCLBase* entryPointsCL,
    const LayerCLBase* exitPointsCL, const uvec2& outportDim,
    const VECTOR_CLASS<cl::Event>* waitForEvents /*= nullptr*/, cl::Event* event /*= nullptr*/) {
    size2_t localWorkGroupSize(workGroupSize_);
    size2_t globalWorkGroupSize(getGlobalWorkGroupSize(outportDim.x, localWorkGroupSize.x),
                                getGlobalWorkGroupSize(outportDim.y, localWorkGroupSize.y));

    try {
        cl_uint arg = 0;
        kernel_->setArg(arg++, NDCToTextureMat);
        kernel_->setArg(arg++, worldToTextureMat);
        kernel_->setArg(arg++, *vertices);
        kernel_->setArg(arg++, *indices);
        kernel_->setArg(arg++, nIndices);
        kernel_->setArg(arg++, *entryPointsCL);
        kernel_->setArg(arg++, *exitPointsCL);
        OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(
            *kernel_, cl::NullRange, globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}

}  // namespace
