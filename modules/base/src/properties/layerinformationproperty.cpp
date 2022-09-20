/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>               // for operator""_help
#include <inviwo/core/datastructures/image/imagetypes.h>  // for operator<<
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/properties/compositeproperty.h>     // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>    // for ConstraintBehavior, ConstraintB...
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/properties/ordinalproperty.h>       // for OrdinalProperty, IntSizeTProperty
#include <inviwo/core/properties/propertysemantics.h>     // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>        // for StringProperty
#include <inviwo/core/properties/templateproperty.h>      // for TemplateProperty
#include <inviwo/core/properties/valuewrapper.h>          // for PropertySerializationMode, Prop...
#include <inviwo/core/util/foreacharg.h>                  // for for_each_in_tuple
#include <inviwo/core/util/formats.h>                     // for DataFormatBase
#include <inviwo/core/util/stringconversion.h>            // for toString

#include <cstddef>                                        // for size_t
#include <limits>                                         // for numeric_limits<>::type, numeric...
#include <tuple>                                          // for tuple, tie

namespace inviwo {

const std::string LayerInformationProperty::classIdentifier = "org.inviwo.LayerInformationProperty";
std::string LayerInformationProperty::getClassIdentifier() const { return classIdentifier; }

LayerInformationProperty::LayerInformationProperty(std::string_view identifier,
                                                   std::string_view displayName,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : CompositeProperty(
          identifier, displayName,
          "A CompositeProperty holding properties to show a information about an image layer"_help,
          invalidationLevel, semantics)
    , layerType_("layerType", "Layer Type", "Type of the layer (Color, Depth, or Picking)"_help)
    , format_("format", "Format", "Underlying data format of the layer"_help, "")
    , channels_("channels", "Channels", "Number of different channels in the layer"_help, 0,
                {0, ConstraintBehavior::Immutable},
                {std::numeric_limits<size_t>::max(), ConstraintBehavior::Immutable}, 1,
                InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , swizzleMask_(
          "swizzleMask", "Swizzle Mask",
          "The swizzle mask is used when sampling the layer for example in OpenGL shaders"_help)
    , interpolation_(
          "interpolation", "Interpolation",
          "Type of interpolation which is used when sampling the layer for example in OpenGL shaders"_help)
    , wrapping_("wrapping", "Wrapping",
                "Defines the wrapping of texture coordinates (Clamp, Repeat, or Mirror)"_help) {

    util::for_each_in_tuple(
        [&](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            addProperty(e);
        },
        std::tie(layerType_, format_, channels_, swizzleMask_, interpolation_, wrapping_));
}

LayerInformationProperty::LayerInformationProperty(const LayerInformationProperty& rhs)
    : CompositeProperty(rhs)
    , layerType_(rhs.layerType_)
    , format_(rhs.format_)
    , channels_(rhs.channels_)
    , swizzleMask_(rhs.swizzleMask_)
    , interpolation_(rhs.interpolation_)
    , wrapping_(rhs.wrapping_) {

    addProperties(layerType_, format_, channels_, swizzleMask_, interpolation_, wrapping_);
}

LayerInformationProperty* LayerInformationProperty::clone() const {
    return new LayerInformationProperty(*this);
}

void LayerInformationProperty::updateFromLayer(const Layer& layer) {
    layerType_.set(toString(layer.getLayerType()));
    format_.set(layer.getDataFormat()->getString());
    channels_.set(layer.getDataFormat()->getComponents());
    swizzleMask_.set(toString(layer.getSwizzleMask()));
    interpolation_.set(toString(layer.getInterpolation()));
    wrapping_.set(toString(layer.getWrapping()));
}

}  // namespace inviwo
