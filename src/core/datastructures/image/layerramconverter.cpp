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

#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

namespace inviwo {

LayerDisk2RAMConverter::LayerDisk2RAMConverter()
    : RepresentationConverterType<LayerRAM>()
{}

LayerDisk2RAMConverter::~LayerDisk2RAMConverter() {}

/**
 * Converts a LayerDisk representation to a RAM representation. This is done if a Image
 * has a representation of LayerDisk and a LayerRAM representation is required. This is
 * used in data.h.
 *
 * @param source is the input representation that is to be converted.
 * @return the imageRAM representation of the file. Returns nullptr if source is not a
 * LayerDisk object.
 **/
DataRepresentation* LayerDisk2RAMConverter::createFrom(const DataRepresentation* source) {
    const LayerDisk* layerDisk = static_cast<const LayerDisk*>(source);

    void* data = layerDisk->readData();

    switch (layerDisk->getDataFormat()->getId()) {
#define DataFormatIdMacro(i) case DataFormatEnums::i: return new LayerRAM_##i(static_cast<Data##i::type*>(data), layerDisk->getDimensions(), layerDisk->getLayerType());
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogError("Cannot convert format from Disk to RAM:" << layerDisk->getDataFormat()->getString());
    }

    return nullptr;
}

void LayerDisk2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const LayerDisk* layerSrc = static_cast<const LayerDisk*>(source);
    LayerRAM* layerDst = static_cast<LayerRAM*>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions())
        layerDst->setDimensions(layerSrc->getDimensions());

    layerSrc->readDataInto(layerDst->getData());
}

} // namespace
