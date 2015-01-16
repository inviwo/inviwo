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

#include <modules/opencl/image/layerclconverter.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

LayerRAM2CLConverter::LayerRAM2CLConverter()
    : RepresentationConverterType<LayerCL>()
{}

DataRepresentation* LayerRAM2CLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const LayerRAM* layerRAM = static_cast<const LayerRAM*>(source);
    uvec2 dimensions = layerRAM->getDimensions();;
    const void* data = layerRAM->getData();
    destination = new LayerCL(dimensions, layerRAM->getLayerType(), layerRAM->getDataFormat(), data);
    return destination;
}
void LayerRAM2CLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const LayerRAM* layerSrc = static_cast<const LayerRAM*>(source);
    LayerCL* layerDst = static_cast<LayerCL*>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->resize(layerSrc->getDimensions());
    }

    layerDst->upload(layerSrc->getData());
}

LayerCL2RAMConverter::LayerCL2RAMConverter()
    : RepresentationConverterType<LayerRAM>()
{}


DataRepresentation* LayerCL2RAMConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const LayerCL* layerCL = static_cast<const LayerCL*>(source);
    uvec2 dimensions = layerCL->getDimensions();
    destination = createLayerRAM(dimensions, layerCL->getLayerType(), layerCL->getDataFormat());

    if (destination) {
        LayerRAM* layerRAM = static_cast<LayerRAM*>(destination);
        layerCL->download(layerRAM->getData());
        //const cl::CommandQueue& queue = OpenCL::getInstance()->getQueue();
        //queue.enqueueReadLayer(layerCL->getLayer(), true, glm::svec3(0), glm::svec3(dimensions, 1), 0, 0, layerRAM->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void LayerCL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const LayerCL* layerSrc = static_cast<const LayerCL*>(source);
    LayerRAM* layerDst = static_cast<LayerRAM*>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->resize(layerSrc->getDimensions());
    }

    layerSrc->download(layerDst->getData());
}

} // namespace
