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

#include <modules/base/properties/layerinformationproperty.h>

#include <inviwo/core/datastructures/image/layer.h>

namespace inviwo {

const std::string LayerInformationProperty::classIdentifier = "org.inviwo.LayerInformationProperty";
std::string LayerInformationProperty::getClassIdentifier() const { return classIdentifier; }

LayerInformationProperty::LayerInformationProperty(std::string identifier, std::string displayName,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , layerType_("layerType", "Layer Type")
    , format_("format", "Format", "")
    , channels_("channels", "Channels", 0, 0, std::numeric_limits<size_t>::max(), 1,
                InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , swizzleMask_("swizzleMask", "Swizzle Mask") {

    util::for_each_in_tuple(
        [&](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            this->addProperty(e);
        },
        std::tie(layerType_, format_, channels_, swizzleMask_));
}

LayerInformationProperty::LayerInformationProperty(const LayerInformationProperty& rhs)
    : CompositeProperty(rhs)
    , layerType_(rhs.layerType_)
    , format_(rhs.format_)
    , channels_(rhs.channels_)
    , swizzleMask_(rhs.swizzleMask_) {
    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); },
                            std::tie(layerType_, format_, channels_, swizzleMask_));
}

LayerInformationProperty* LayerInformationProperty::clone() const {
    return new LayerInformationProperty(*this);
}

void LayerInformationProperty::updateFromLayer(const Layer& layer) {
    layerType_.set(toString(layer.getLayerType()));
    format_.set(layer.getDataFormat()->getString());
    channels_.set(layer.getDataFormat()->getComponents());
    swizzleMask_.set(toString(layer.getSwizzleMask()));
}

}  // namespace inviwo
