/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>                    // for operator""_help
#include <inviwo/core/datastructures/image/image.h>            // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>       // for ImageType, operator<<, Ima...
#include <inviwo/core/properties/compositeproperty.h>          // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>         // for ConstraintBehavior, Constr...
#include <inviwo/core/properties/invalidationlevel.h>          // for InvalidationLevel, Invalid...
#include <inviwo/core/properties/ordinalproperty.h>            // for OrdinalProperty, DoublePro...
#include <inviwo/core/properties/propertysemantics.h>          // for PropertySemantics, Propert...
#include <inviwo/core/properties/stringproperty.h>             // for StringProperty
#include <inviwo/core/properties/templateproperty.h>           // for TemplateProperty
#include <inviwo/core/properties/valuewrapper.h>               // for PropertySerializationMode
#include <inviwo/core/util/foreacharg.h>                       // for for_each_in_tuple
#include <inviwo/core/util/glmvec.h>                           // for size2_t
#include <inviwo/core/util/stringconversion.h>                 // for toString
#include <modules/base/properties/layerinformationproperty.h>  // for LayerInformationProperty

#include <cstddef>  // for size_t
#include <limits>   // for numeric_limits<>::type
#include <memory>   // for make_unique, unique_ptr
#include <vector>   // for vector

#include <glm/vec2.hpp>  // for vec, vec<>::(anonymous)

namespace inviwo {
class Layer;

const std::string ImageInformationProperty::classIdentifier = "org.inviwo.ImageInformationProperty";
std::string ImageInformationProperty::getClassIdentifier() const { return classIdentifier; }

ImageInformationProperty::ImageInformationProperty(std::string_view identifier,
                                                   std::string_view displayName,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, "Information about an image"_help,
                        invalidationLevel, semantics)
    , dimensions_("dimensions", "Dimensions", "Image dimensions"_help, size2_t(0),
                  {size2_t(0), ConstraintBehavior::Immutable},
                  {size2_t(std::numeric_limits<size_t>::max()), ConstraintBehavior::Immutable},
                  size2_t(1), InvalidationLevel::Valid, PropertySemantics("Text"))
    , aspectRatio_("aspectRatio", "Aspect Ratio", 0.0, {0.0, ConstraintBehavior::Immutable},
                   {std::numeric_limits<double>::max(), ConstraintBehavior::Immutable}, 0.1,
                   InvalidationLevel::Valid, PropertySemantics::Text)
    , imageType_("imageType", "Image Type")
    , numColorLayers_("numColorLayers", "Color Layers", "Number of color layers"_help, 0,
                      {0, ConstraintBehavior::Immutable},
                      {std::numeric_limits<size_t>::max(), ConstraintBehavior::Immutable}, 1,
                      InvalidationLevel::Valid, PropertySemantics::Text)
    , layers_("layers", "Layers", "Detailed information on all layers in the image"_help) {
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
    , aspectRatio_(rhs.aspectRatio_)
    , imageType_(rhs.imageType_)
    , numColorLayers_(rhs.numColorLayers_)
    , layers_(rhs.layers_) {
    addProperties(dimensions_, aspectRatio_, imageType_, numColorLayers_, layers_);
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
    aspectRatio_.set(static_cast<double>(dim.x) / dim.y);
    imageType_.set(toString(type));
    numColorLayers_.set(image.getNumberOfColorLayers());

    const size_t numLayers =
        image.getNumberOfColorLayers() + (hasDepth ? 1 : 0) + (hasPicking ? 1 : 0);

    while (layers_.size() < numLayers) {
        auto p = std::make_unique<LayerInformationProperty>(
            "layer" + std::to_string(layers_.size()), "");
        p->setReadOnly(true);
        p->setSerializationMode(PropertySerializationMode::None);
        layers_.addProperty(p.release());
    }
    // remove additional layers
    while (layers_.size() > numLayers) {
        layers_.removeProperty(layers_.size() - 1);
    }

    auto updateProperty = [](auto* prop, const Layer& layer, const std::string& name) {
        auto p = static_cast<LayerInformationProperty*>(prop);
        p->setDisplayName(name);
        p->updateForNewLayer(layer, util::OverwriteState::Yes);
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
