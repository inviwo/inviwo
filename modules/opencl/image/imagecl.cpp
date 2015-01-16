/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <modules/opencl/image/imagecl.h>

namespace inviwo {

ImageCL::ImageCL()
    : ImageRepresentation(), layerCL_(NULL) {}

ImageCL::ImageCL(const ImageCL& rhs)
    : ImageRepresentation(rhs) {}

ImageCL::~ImageCL() {
}

ImageCL* ImageCL::clone() const {
    return new ImageCL(*this);
}

void ImageCL::initialize() {
}

void ImageCL::deinitialize() {
}

LayerCL* ImageCL::getLayerCL() {
    return layerCL_;
}

const LayerCL* ImageCL::getLayerCL() const {
    return layerCL_;
}

bool ImageCL::copyAndResizeRepresentation(DataRepresentation* targetRep) const {
    ImageCL* targetCL = dynamic_cast<ImageCL*>(targetRep);

    if (!targetCL) return false;

    return this->getLayerCL()->copyAndResizeLayer(targetCL->getLayerCL());
}

void ImageCL::update(bool editable) {
    //TODO: Convert more then just first color layer
    layerCL_ = NULL;
    Image *owner = this->getOwner();

    if (editable) {
        layerCL_ = owner->getColorLayer()->getEditableRepresentation<LayerCL>();
    } else {
        layerCL_ = const_cast<LayerCL*>(owner->getColorLayer()->getRepresentation<LayerCL>());
    }

    if(layerCL_->getDataFormat() != getOwner()->getDataFormat()){
        owner->getColorLayer()->setDataFormat(layerCL_->getDataFormat());
        owner->getColorLayer()->setDimensions(layerCL_->getDimensions());
    }
}

} // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::ImageCL& value) {
    return setArg(index, value.getLayerCL()->get());
}

} // namespace cl
