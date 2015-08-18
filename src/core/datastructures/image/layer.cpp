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

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

Layer::Layer(size2_t dimensions, const DataFormatBase* format, LayerType type)
    : Data(format), StructuredGridEntity<2>(dimensions), layerType_(type) {}

Layer::Layer(LayerRepresentation* in)
    : Data(in->getDataFormat())
    , StructuredGridEntity<2>(in->getDimensions())
    , layerType_(in->getLayerType()) {
    clearRepresentations();
    addRepresentation(in);
}

Layer::Layer(const Layer& rhs)
    : Data(rhs), StructuredGridEntity<2>(rhs), layerType_(rhs.layerType_) {}

Layer& Layer::operator=(const Layer& that) {
    if (this != &that) {
        Data::operator=(that);
        StructuredGridEntity<2>::operator=(that);
        layerType_ = that.layerType_;
    }

    return *this;
}

Layer* Layer::clone() const { return new Layer(*this); }

Layer::~Layer() {
    // Representations are deleted by Data destructor.
}

size2_t Layer::getDimensions() const {
    if (hasRepresentations() && lastValidRepresentation_) {
        size2_t dim = static_cast<LayerRepresentation*>(lastValidRepresentation_)->getDimensions();
        if (dim != size2_t(0)) return dim;
    }
    return StructuredGridEntity<2>::getDimensions();
}

void Layer::setDimensions(const size2_t& dim) {
    StructuredGridEntity<2>::setDimensions(dim);

    if (lastValidRepresentation_) {
        // Resize last valid representation
        static_cast<LayerRepresentation*>(lastValidRepresentation_)->setDimensions(dim);

        // and remove the other ones
        util::erase_remove_if(representations_, [this](DataRepresentation* repr) {
            if (repr != lastValidRepresentation_) {
                delete repr;
                return true;
            } else {
                return false;
            }
        });
        setAllRepresentationsAsInvalid();
        // Set the remaining representation as valid.
        // Solves issue where the layer will try to update 
        // the remaining representation with itself when getRepresentation of the same type is called
        setRepresentationAsValid(0);
    }
}

void Layer::copyRepresentationsTo(Layer* targetLayer) {
    // TODO: And also need to be tested on multiple representations_ such as LayerRAM, LayerDisk
    // TODO: optimize the code

    auto& targets = targetLayer->representations_;
    bool copyDone = false;

    for (int i = 0; i < static_cast<int>(representations_.size()); i++) {
        if (isRepresentationValid(i)) {
            auto sourceRepr = static_cast<LayerRepresentation*>(representations_[i]);

            for (int j = 0; j < static_cast<int>(targets.size()); j++) {
                auto targetRepr = static_cast<LayerRepresentation*>(targets[j]);

                if (typeid(*sourceRepr) == typeid(*targetRepr)) {
                    if (sourceRepr->copyRepresentationsTo(targetRepr)) {
                        targetLayer->setRepresentationAsValid(j);
                        targetLayer->lastValidRepresentation_ = targets[j];
                        copyDone = true;
                    }
                }
            }
        }
    }

    if (!copyDone) {  // Fallback
        ivwAssert(lastValidRepresentation_ != nullptr, "Last valid representation is expected.");
        targetLayer->setAllRepresentationsAsInvalid();
        targetLayer->createDefaultRepresentation();

        auto lastValidRepresentation = dynamic_cast<LayerRepresentation*>(lastValidRepresentation_);
        auto clone = dynamic_cast<LayerRepresentation*>(lastValidRepresentation_->clone());

        targetLayer->addRepresentation(clone);
        targetLayer->setDimensions(targetLayer->getDimensions());

        if (lastValidRepresentation->copyRepresentationsTo(clone)) {
            targetLayer->setRepresentationAsValid(
                static_cast<int>(targetLayer->representations_.size()) - 1);
            targetLayer->lastValidRepresentation_ = clone;
        }
    }
}

LayerType Layer::getLayerType() const { return layerType_; }

DataRepresentation* Layer::createDefaultRepresentation() {
    return createLayerRAM(getDimensions(), getLayerType(), getDataFormat());
}

}  // namespace
