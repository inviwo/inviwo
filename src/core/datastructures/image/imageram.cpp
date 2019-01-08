/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

ImageRAM::ImageRAM() = default;

ImageRAM::ImageRAM(const ImageRAM& rhs) : ImageRepresentation(rhs) { update(true); }

ImageRAM* ImageRAM::clone() const { return new ImageRAM(*this); }
ImageRAM& ImageRAM::operator=(const ImageRAM& that) {
    if (this != &that) {
        ImageRepresentation::operator=(that);
        update(true);
    }

    return *this;
}
ImageRAM::~ImageRAM() = default;

size2_t ImageRAM::getDimensions() const { return colorLayersRAM_.front()->getDimensions(); }

bool ImageRAM::copyRepresentationsTo(ImageRepresentation* targetRep) const {
    const ImageRAM* source = this;
    ImageRAM* target = dynamic_cast<ImageRAM*>(targetRep);

    if (!target) return false;

    // Copy and resize color layers
    size_t minSize = std::min(source->getNumberOfColorLayers(), target->getNumberOfColorLayers());

    for (size_t i = 0; i < minSize; ++i) {
        if (!source->getColorLayerRAM(i)->copyRepresentationsTo(target->getColorLayerRAM(i)))
            return false;
    }

    // Copy and resize depth layer
    if (source->getDepthLayerRAM() && target->getDepthLayerRAM())
        if (!source->getDepthLayerRAM()->copyRepresentationsTo(target->getDepthLayerRAM()))
            return false;

    // Copy and resize picking layer
    if (source->getPickingLayerRAM() && target->getPickingLayerRAM())
        if (!source->getPickingLayerRAM()->copyRepresentationsTo(target->getPickingLayerRAM()))
            return false;

    return true;
}

size_t ImageRAM::priority() const { return 200; }

void ImageRAM::update(bool editable) {
    colorLayersRAM_.clear();
    depthLayerRAM_ = nullptr;
    pickingLayerRAM_ = nullptr;

    if (editable) {
        auto owner = static_cast<Image*>(this->getOwner());
        for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
            colorLayersRAM_.push_back(
                owner->getColorLayer(i)->getEditableRepresentation<LayerRAM>());
        }

        if (auto depthLayer = owner->getDepthLayer()) {
            depthLayerRAM_ = depthLayer->getEditableRepresentation<LayerRAM>();
        }

        if (auto pickingLayer = owner->getPickingLayer()) {
            pickingLayerRAM_ = pickingLayer->getEditableRepresentation<LayerRAM>();
        }
    } else {
        auto owner = static_cast<const Image*>(this->getOwner());
        for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
            colorLayersRAM_.push_back(
                const_cast<LayerRAM*>(owner->getColorLayer(i)->getRepresentation<LayerRAM>()));
        }

        if (auto depthLayer = owner->getDepthLayer()) {
            depthLayerRAM_ = const_cast<LayerRAM*>(depthLayer->getRepresentation<LayerRAM>());
        }

        if (auto pickingLayer = owner->getPickingLayer()) {
            pickingLayerRAM_ = const_cast<LayerRAM*>(pickingLayer->getRepresentation<LayerRAM>());
        }
    }
}

LayerRAM* ImageRAM::getColorLayerRAM(size_t idx) { return colorLayersRAM_.at(idx); }

LayerRAM* ImageRAM::getDepthLayerRAM() { return depthLayerRAM_; }

LayerRAM* ImageRAM::getPickingLayerRAM() { return pickingLayerRAM_; }

size_t ImageRAM::getNumberOfColorLayers() const { return colorLayersRAM_.size(); }

std::type_index ImageRAM::getTypeIndex() const { return std::type_index(typeid(ImageRAM)); }

const LayerRAM* ImageRAM::getColorLayerRAM(size_t idx) const { return colorLayersRAM_.at(idx); }

const LayerRAM* ImageRAM::getDepthLayerRAM() const { return depthLayerRAM_; }

const LayerRAM* ImageRAM::getPickingLayerRAM() const { return pickingLayerRAM_; }

bool ImageRAM::isValid() const {
    return depthLayerRAM_->isValid() && pickingLayerRAM_->isValid() &&
           util::all_of(colorLayersRAM_, [](const auto& l) { return l->isValid(); });
}

dvec4 ImageRAM::readPixel(size2_t pos, LayerType layer, size_t index) const {
    switch (layer) {
        case LayerType::Depth:
            return depthLayerRAM_->getAsDVec4(pos);
        case LayerType::Picking:
            return pickingLayerRAM_->getAsDVec4(pos);
        case LayerType::Color:
        default:
            return colorLayersRAM_[index]->getAsDVec4(pos);
    }
}

}  // namespace inviwo
