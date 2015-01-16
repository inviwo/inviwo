/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

namespace inviwo {

ImageRAM::ImageRAM()
    : ImageRepresentation() {
}

ImageRAM::ImageRAM(const ImageRAM& rhs)
    : ImageRepresentation(rhs) {
    update(true);
}

ImageRAM* ImageRAM::clone() const {
    return new ImageRAM(*this);
}
ImageRAM& ImageRAM::operator=(const ImageRAM& that) {
    if (this != &that) {
        ImageRepresentation::operator=(that);
        update(true);
    }

    return *this;
}
ImageRAM::~ImageRAM() {
    ImageRAM::deinitialize();
}

void ImageRAM::initialize() {
}

void ImageRAM::deinitialize() {
}

bool ImageRAM::copyAndResizeRepresentation(DataRepresentation* targetRep) const {
    const ImageRAM* source = this;
    ImageRAM* target = dynamic_cast<ImageRAM*>(targetRep);
    ivwAssert(target!=0, "Target representation missing.");
    //Copy and resize color layers
    size_t minSize = std::min(source->getOwner()->getNumberOfColorLayers(), target->getOwner()->getNumberOfColorLayers());

    for (size_t i=0; i<minSize; ++i) {
        if (!source->getColorLayerRAM(i)->copyAndResizeLayer(target->getColorLayerRAM(i)))
            return false;
    }

    //Copy and resize depth layer
    if (source->getDepthLayerRAM() && target->getDepthLayerRAM())
        if (!source->getDepthLayerRAM()->copyAndResizeLayer(target->getDepthLayerRAM()))
            return false;

    //Copy and resize picking layer
    if (source->getPickingLayerRAM() && target->getPickingLayerRAM())
        if (!source->getPickingLayerRAM()->copyAndResizeLayer(target->getPickingLayerRAM()))
            return false;

    return true;
}

void ImageRAM::update(bool editable) {
    colorLayersRAM_.clear();
    depthLayerRAM_ = NULL;
    pickingLayerRAM_ = NULL;
 
    if (editable) {
        Image *owner = this->getOwner();
        for (size_t i=0; i<owner->getNumberOfColorLayers(); ++i){
            colorLayersRAM_.push_back(owner->getColorLayer(i)->getEditableRepresentation<LayerRAM>());
        }

        Layer* depthLayer = owner->getDepthLayer();
        if (depthLayer){
            depthLayerRAM_ = depthLayer->getEditableRepresentation<LayerRAM>();
        }

        Layer* pickingLayer = owner->getPickingLayer();
        if (pickingLayer){
            pickingLayerRAM_ = pickingLayer->getEditableRepresentation<LayerRAM>();
        }
    }
    else {
        const Image *owner = this->getOwner();
        for (size_t i=0; i<owner->getNumberOfColorLayers(); ++i){
            colorLayersRAM_.push_back(const_cast<LayerRAM*>(owner->getColorLayer(i)->getRepresentation<LayerRAM>()));
        }

        const Layer* depthLayer = owner->getDepthLayer();
        if (depthLayer){
            depthLayerRAM_ = const_cast<LayerRAM*>(depthLayer->getRepresentation<LayerRAM>());
        }

        const Layer* pickingLayer = owner->getPickingLayer();
        if (pickingLayer){
            pickingLayerRAM_ = const_cast<LayerRAM*>(pickingLayer->getRepresentation<LayerRAM>());
        }
    }
}

LayerRAM* ImageRAM::getColorLayerRAM(size_t idx) {
    return colorLayersRAM_.at(idx);
}

LayerRAM* ImageRAM::getDepthLayerRAM() {
    return depthLayerRAM_;
}

LayerRAM* ImageRAM::getPickingLayerRAM() {
    return pickingLayerRAM_;
}

const LayerRAM* ImageRAM::getColorLayerRAM(size_t idx) const {
    return colorLayersRAM_.at(idx);
}

const LayerRAM* ImageRAM::getDepthLayerRAM() const {
    return depthLayerRAM_;
}

const LayerRAM* ImageRAM::getPickingLayerRAM() const {
    return pickingLayerRAM_;
}



} // namespace
