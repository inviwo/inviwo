/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/tflookuptable.h>

namespace inviwo {

TFLookupTable::TFLookupTable(TransferFunction& tf, size_t size)
    : tf_{&tf}, size_{size}, invalid_{true}, repr_{}, data_{} {
    tf_->addObserver(this);
}

void TFLookupTable::setSize(size_t size) {
    if (size != size_) {
        size_ = size;
        invalid_ = true;
    }
}
size_t TFLookupTable::getSize() const { return size_; }

const TransferFunction& TFLookupTable::getTransferFunction() const { return *tf_; }
void TFLookupTable::setTransferFunction(TransferFunction& tf) {
    if (tf_ != &tf) {
        tf_->removeObserver(this);
        tf_ = &tf;
        tf_->addObserver(this);
        invalid_ = true;
    }
}

void TFLookupTable::calc() {
    if (!repr_) {
        repr_ = std::make_shared<LayerRAMPrecision<vec4>>(size2_t{size_, 1});
    }
    if (repr_->getDimensions().x != size_) {
        repr_->setDimensions(size2_t{size_, 1});
    }
    if (!data_) {
        data_ = std::make_unique<Layer>(repr_);
    }
    tf_->interpolateAndStoreColors(repr_->getView());
    data_->invalidateAllOther(repr_.get());
    invalid_ = false;
}

void TFLookupTable::onTFPrimitiveAdded(const TFPrimitiveSet&, TFPrimitive&) { invalid_ = true; }
void TFLookupTable::onTFPrimitiveRemoved(const TFPrimitiveSet&, TFPrimitive&) { invalid_ = true; }
void TFLookupTable::onTFPrimitiveChanged(const TFPrimitiveSet&, const TFPrimitive&) {
    invalid_ = true;
}
void TFLookupTable::onTFTypeChanged(const TFPrimitiveSet&, TFPrimitiveSetType) { invalid_ = true; }
void TFLookupTable::onTFMaskChanged(const TFPrimitiveSet&, dvec2) { invalid_ = true; }

}  // namespace inviwo
