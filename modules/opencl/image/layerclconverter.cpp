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
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

std::shared_ptr<LayerCL> LayerRAM2CLConverter::createFrom(
    std::shared_ptr<const LayerRAM> layerRAM) const {
    uvec2 dimensions = layerRAM->getDimensions();
    const void* data = layerRAM->getData();
    return std::make_shared<LayerCL>(dimensions, layerRAM->getLayerType(),
                                     layerRAM->getDataFormat(), data);
}

void LayerRAM2CLConverter::update(std::shared_ptr<const LayerRAM> layerSrc,
                                  std::shared_ptr<LayerCL> layerDst) const {
    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }

    layerDst->upload(layerSrc->getData());
}

std::shared_ptr<LayerRAM> LayerCL2RAMConverter::createFrom(
    std::shared_ptr<const LayerCL> layerCL) const {
    uvec2 dimensions = layerCL->getDimensions();
    auto destination =
        createLayerRAM(dimensions, layerCL->getLayerType(), layerCL->getDataFormat());

    if (destination) {
        layerCL->download(destination->getData());
        // const cl::CommandQueue& queue = OpenCL::getInstance()->getQueue();
        // queue.enqueueReadLayer(layerCL->getLayer(), true, glm::size3_t(0),
        // glm::size3_t(dimensions,
        // 1), 0, 0, layerRAM->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void LayerCL2RAMConverter::update(std::shared_ptr<const LayerCL> layerSrc,
                                  std::shared_ptr<LayerRAM> layerDst) const {
    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }

    layerSrc->download(layerDst->getData());
}

}  // namespace
