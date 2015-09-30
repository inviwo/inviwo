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
    : Data<LayerRepresentation>(format), StructuredGridEntity<2>(dimensions), layerType_(type) {}

Layer::Layer(std::shared_ptr<LayerRepresentation> in)
    : Data<LayerRepresentation>(in->getDataFormat())
    , StructuredGridEntity<2>(in->getDimensions())
    , layerType_(in->getLayerType()) {
    addRepresentation(in);
}

Layer::Layer(const Layer& rhs)
    : Data<LayerRepresentation>(rhs), StructuredGridEntity<2>(rhs), layerType_(rhs.layerType_) {}

Layer& Layer::operator=(const Layer& that) {
    if (this != &that) {
        Data<LayerRepresentation>::operator=(that);
        StructuredGridEntity<2>::operator=(that);
        layerType_ = that.layerType_;
    }
    return *this;
}

Layer* Layer::clone() const { return new Layer(*this); }

Layer::~Layer() {}

size2_t Layer::getDimensions() const {
    if (hasRepresentations() && lastValidRepresentation_) {
        size2_t dim = lastValidRepresentation_->getDimensions();
        if (dim != size2_t(0)) return dim;
    }
    return StructuredGridEntity<2>::getDimensions();
}

void Layer::setDimensions(const size2_t& dim) {
    StructuredGridEntity<2>::setDimensions(dim);

    if (lastValidRepresentation_) {
        // Resize last valid representation
        lastValidRepresentation_->setDimensions(dim);
        removeOtherRepresentations(lastValidRepresentation_.get());
    }
}

void Layer::copyRepresentationsTo(Layer* targetLayer) {
    // TODO: And also need to be tested on multiple representations_ such as LayerRAM, LayerDisk
    // TODO: optimize the code

    auto& targets = targetLayer->representations_;
    bool copyDone = false;

    for (auto& sourceElem : representations_) {
        auto sourceRepr = sourceElem.second;
        if (sourceRepr->isValid()) {
            for (auto& targetElem : targets) {
                auto targetRepr = targetElem.second;
                if (typeid(*sourceRepr) == typeid(*targetRepr)) {
                    if (sourceRepr->copyRepresentationsTo(targetRepr.get())) {
                        targetRepr->setValid(true);
                        targetLayer->lastValidRepresentation_ = targetElem.second;
                        copyDone = true;
                    }
                }
            }
        }
    }

    if (!copyDone) {  // Fallback
        ivwAssert(lastValidRepresentation_, "Last valid representation is expected.");
        for (auto& targetElem : targets) targetElem.second->setValid(false);
        targetLayer->createDefaultRepresentation();

        auto clone = std::shared_ptr<LayerRepresentation>(lastValidRepresentation_->clone());
        targetLayer->addRepresentation(clone);
        targetLayer->setDimensions(targetLayer->getDimensions());

        if (lastValidRepresentation_->copyRepresentationsTo(clone.get())) {
            clone->setValid(true);
            targetLayer->lastValidRepresentation_ = clone;
        }
    }
}

LayerType Layer::getLayerType() const { return layerType_; }

std::shared_ptr<LayerRepresentation> Layer::createDefaultRepresentation() const {
    return createLayerRAM(getDimensions(), getLayerType(), getDataFormat());
}

}  // namespace
