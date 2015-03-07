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

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/io/datawriter.h>

namespace inviwo {

LayerRAM::LayerRAM(uvec2 dimensions, LayerType type, const DataFormatBase* format)
    : LayerRepresentation(dimensions, type, format), data_(nullptr) {}

LayerRAM::LayerRAM(const LayerRAM& rhs)
    : LayerRepresentation(rhs) {
}

LayerRAM& LayerRAM::operator=(const LayerRAM& that) {
    if (this != &that)
        LayerRepresentation::operator=(that);

    return *this;
}

LayerRAM::~LayerRAM() {
}

bool LayerRAM::copyAndResizeLayer(DataRepresentation* targetLayerRam) const {
    return Canvas::generalLayerWriter_->writeDataToRepresentation(this, targetLayerRam);
}

void LayerRAM::setDimensions(uvec2 dimensions) {
    resize(dimensions);
}

void* LayerRAM::getData() {
    return data_;
}

const void* LayerRAM::getData() const {
    return data_;
}

void LayerRAM::setData(void* data) {
    deinitialize();
    data_ = data;
}

LayerRAM* createLayerRAM(const uvec2& dimensions, LayerType type, const DataFormatBase* format) {
    switch (format->getId()) {
        case DataFormatEnums::NOT_SPECIALIZED:
            LogErrorCustom("createLayerRAM", "Invalid format");
            return nullptr;
#define DataFormatIdMacro(i)                                           \
    case DataFormatEnums::i:                                           \
        return new LayerRAMPrecision<Data##i::type>(dimensions, type); \
        break;
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogErrorCustom("createLayerRAM", "Invalid format or not implemented");
            return nullptr;
    }

    return nullptr;
}

} // namespace
