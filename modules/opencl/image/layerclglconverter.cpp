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

#include <modules/opencl/image/layerclconverter.h>
#include <modules/opencl/image/layerclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

std::shared_ptr<DataRepresentation> LayerCLGL2RAMConverter::createFrom(
    std::shared_ptr<const DataRepresentation> source) const {
    auto layerCLGL = std::static_pointer_cast<const LayerCLGL>(source);
    uvec2 dimensions = layerCLGL->getDimensions();
    auto destination =
        createLayerRAM(dimensions, layerCLGL->getLayerType(), layerCLGL->getDataFormat());

    if (destination) {
        layerCLGL->getTexture()->download(destination->getData());
        // const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue();
        // queue.enqueueReadLayer(layerCL->getLayer(), true, glm::size3_t(0),
        // glm::size3_t(dimensions, 1), 0, 0, layerRAM->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void LayerCLGL2RAMConverter::update(std::shared_ptr<const DataRepresentation> source,
                                    std::shared_ptr<DataRepresentation> destination) const {
    auto layerSrc = std::static_pointer_cast<const LayerCLGL>(source);
    auto layerDst = std::static_pointer_cast<LayerRAM>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }

    layerSrc->getTexture()->download(layerDst->getData());
}

std::shared_ptr<DataRepresentation> LayerCLGL2GLConverter::createFrom(
    std::shared_ptr<const DataRepresentation> source) const {
    auto src = std::static_pointer_cast<const LayerCLGL>(source);
    return std::make_shared<LayerGL>(src->getDimensions(), src->getLayerType(),
                                     src->getDataFormat(), src->getTexture());
}

void LayerCLGL2GLConverter::update(std::shared_ptr<const DataRepresentation> source,
                                   std::shared_ptr<DataRepresentation> destination) const {
    // Do nothing since they share data
}

std::shared_ptr<DataRepresentation> LayerCLGL2CLConverter::createFrom(
    std::shared_ptr<const DataRepresentation> source) const {
#ifdef IVW_DEBUG
    LogWarn("Performance warning: Use shared CLGL representation instead of CL ");
#endif
    auto src = std::static_pointer_cast<const LayerCLGL>(source);
    auto destination =
        std::make_shared<LayerCL>(src->getDimensions(), src->getLayerType(), src->getDataFormat());
    {
        SyncCLGL glSync;
        src->aquireGLObject(glSync.getGLSyncEvent());
        OpenCL::getPtr()->getQueue().enqueueCopyImage(src->get(), destination->get(),
                                                      glm::size3_t(0), glm::size3_t(0),
                                                      glm::size3_t(src->getDimensions(), 1));
        src->releaseGLObject(nullptr, glSync.getLastReleaseGLEvent());
    }
    return destination;
}

void LayerCLGL2CLConverter::update(std::shared_ptr<const DataRepresentation> source,
                                   std::shared_ptr<DataRepresentation> destination) const {
    auto src = std::static_pointer_cast<const LayerCLGL>(source);
    auto dst = std::static_pointer_cast<LayerCL>(destination);

    if (src->getDimensions() != dst->getDimensions()) {
        dst->setDimensions(src->getDimensions());
    }

    {
        SyncCLGL glSync;
        src->aquireGLObject(glSync.getGLSyncEvent());
        OpenCL::getPtr()->getQueue().enqueueCopyImage(src->get(), dst->get(), glm::size3_t(0),
                                                      glm::size3_t(0),
                                                      glm::size3_t(src->getDimensions(), 1));
        src->releaseGLObject(nullptr, glSync.getLastReleaseGLEvent());
    }
}

std::shared_ptr<DataRepresentation> LayerGL2CLGLConverter::createFrom(
    std::shared_ptr<const DataRepresentation> source) const {
    auto layerGL = std::static_pointer_cast<const LayerGL>(source);
    return std::make_shared<LayerCLGL>(layerGL->getDimensions(), layerGL->getLayerType(),
                                       layerGL->getDataFormat(), layerGL->getTexture());
}

void LayerGL2CLGLConverter::update(std::shared_ptr<const DataRepresentation> source,
                                   std::shared_ptr<DataRepresentation> destination) const {
    auto layerSrc = std::static_pointer_cast<const LayerGL>(source);
    auto layerDst = std::static_pointer_cast<LayerCLGL>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }
}

}  // namespace
