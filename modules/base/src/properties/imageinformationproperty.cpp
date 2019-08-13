/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/properties/imageinformationproperty.h>
#include <modules/base/properties/layerinformationproperty.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

const std::string ImageInformationProperty::classIdentifier = "org.inviwo.ImageInformationProperty";
std::string ImageInformationProperty::getClassIdentifier() const { return classIdentifier; }

ImageInformationProperty::ImageInformationProperty(std::string identifier, std::string displayName,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , dimensions_("dimensions", "Dimensions", size2_t(0), size2_t(0),
                  size2_t(std::numeric_limits<size_t>::max()), size2_t(1), InvalidationLevel::Valid,
                  PropertySemantics("Text"))
    , imageType_("imageType", "Image Type")
    , numColorLayers_("numColorLayers", "Number of Color Layers", 0, 0,
                      std::numeric_limits<size_t>::max(), 1, InvalidationLevel::Valid,
                      PropertySemantics::Text)
    , layers_("layers", "Layers") {
    util::for_each_in_tuple(
        [&](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            this->addProperty(e);
        },
        props());

    addProperty(layers_);
    layers_.setCollapsed(true);

    layers_.setCurrentStateAsDefault();
}

ImageInformationProperty::ImageInformationProperty(const ImageInformationProperty& rhs)
    : CompositeProperty(rhs)
    , dimensions_(rhs.dimensions_)
    , imageType_(rhs.imageType_)
    , numColorLayers_(rhs.numColorLayers_)
    , layers_(rhs.layers_) {
    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());
    addProperty(layers_);
}

ImageInformationProperty* ImageInformationProperty::clone() const {
    return new ImageInformationProperty(*this);
}

void ImageInformationProperty::updateForNewImage(const Image& image) {
    const auto dim = image.getDimensions();

    const bool hasDepth = image.getDepthLayer() != nullptr;
    const bool hasPicking = image.getPickingLayer() != nullptr;
    const ImageType type = [hasDepth, hasPicking]() {
        if (hasDepth && hasPicking) {
            return ImageType::ColorDepthPicking;
        } else if (hasDepth) {
            return ImageType::ColorDepth;
        } else if (hasPicking) {
            return ImageType::ColorPicking;
        } else {
            return ImageType::ColorOnly;
        }
    }();

    dimensions_.set(dim);
    imageType_.set(toString(type));
    numColorLayers_.set(image.getNumberOfColorLayers());

    const size_t numLayers =
        image.getNumberOfColorLayers() + (hasDepth ? 1 : 0) + (hasPicking ? 1 : 0);

    while (layers_.size() < numLayers) {
        auto p = std::make_unique<LayerInformationProperty>(
            "layer" + std::to_string(layers_.size()), "");
        p->setReadOnly(true);
        layers_.addProperty(p.release());
    }
    // remove additional layers
    while (layers_.size() > numLayers) {
        layers_.removeProperty(layers_.size() - 1);
    }

    auto updateProperty = [](auto* prop, const Layer& layer, const std::string& name) {
        auto p = static_cast<LayerInformationProperty*>(prop);
        p->setDisplayName(name);
        p->updateFromLayer(layer);
    };

    // update layer information
    const auto& layerprops = layers_.getProperties();
    for (size_t i = 0; i < image.getNumberOfColorLayers(); ++i) {
        updateProperty(layerprops[i], *image.getColorLayer(i),
                       "Color Layer " + std::to_string(i + 1));
    }
    size_t layerIndex = image.getNumberOfColorLayers();
    if (hasDepth) {
        updateProperty(layerprops[layerIndex++], *image.getDepthLayer(), "Depth Layer");
    }
    if (hasPicking) {
        updateProperty(layerprops[layerIndex++], *image.getPickingLayer(), "Picking Layer");
    }

    util::for_each_in_tuple([&](auto& e) { e.setCurrentStateAsDefault(); }, props());
}

}  // namespace inviwo
