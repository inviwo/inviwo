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
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriter.h>

namespace inviwo {

LayerRAM::LayerRAM(size2_t dimensions, LayerType type, const DataFormatBase* format)
    : LayerRepresentation(dimensions, type, format) {}

LayerRAM::LayerRAM(const LayerRAM& rhs) : LayerRepresentation(rhs) {}

LayerRAM& LayerRAM::operator=(const LayerRAM& that) {
    if (this != &that) LayerRepresentation::operator=(that);
    return *this;
}

LayerRAM::~LayerRAM() {}

bool LayerRAM::copyRepresentationsTo(DataRepresentation* targetLayerRam) const {
    static DataWriterType<Layer>* layerWriter_ = InviwoApplication::getPtr()
                                                     ->getDataWriterFactory()
                                                     ->getWriterForTypeAndExtension<Layer>("png")
                                                     .release();

    return layerWriter_->writeDataToRepresentation(this, targetLayerRam);
}

std::type_index LayerRAM::getTypeIndex() const {
    return std::type_index(typeid(LayerRAM));
}

}  // namespace
