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

#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/image/layerclgl.h>

namespace inviwo {

ImageCLGL::ImageCLGL() : ImageRepresentation(), layerCLGL_(nullptr) {}

ImageCLGL::ImageCLGL(const ImageCLGL& rhs) : ImageRepresentation(rhs) {}

ImageCLGL::~ImageCLGL() {}

ImageCLGL* ImageCLGL::clone() const { return new ImageCLGL(*this); }

LayerCLGL* ImageCLGL::getLayerCL() { return layerCLGL_; }

const LayerCLGL* ImageCLGL::getLayerCL() const { return layerCLGL_; }

bool ImageCLGL::copyRepresentationsTo(DataRepresentation* targetRep) const {
    ImageCLGL* targetCLGL = dynamic_cast<ImageCLGL*>(targetRep);

    if (!targetCLGL) return false;

    return layerCLGL_->copyRepresentationsTo(targetCLGL->getLayerCL());
}

std::type_index ImageCLGL::getTypeIndex() const { return std::type_index(typeid(ImageCLGL)); }

void ImageCLGL::update(bool editable) {
    // TODO: Convert more then just first color layer
    layerCLGL_ = nullptr;
    auto owner = static_cast<Image*>(this->getOwner());

    if (editable) {
        layerCLGL_ = owner->getColorLayer()->getEditableRepresentation<LayerCLGL>();
    } else {
        layerCLGL_ = const_cast<LayerCLGL*>(owner->getColorLayer()->getRepresentation<LayerCLGL>());
    }
}

}  // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::ImageCLGL& value) {
    return setArg(index, value.getLayerCL()->get());
}

}  // namespace cl