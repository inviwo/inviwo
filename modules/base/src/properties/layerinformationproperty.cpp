/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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
#include <inviwo/core/util/glm.h>                         // for filled
#include <inviwo/core/util/glmmat.h>                      // for mat2
#include <inviwo/core/util/glmvec.h>                      // for vec2
#include <inviwo/core/util/zip.h>

#include <cstddef>  // for size_t
#include <limits>   // for numeric_limits<>::type, numeric...
#include <tuple>    // for tuple, tie

namespace inviwo {

const std::string LayerInformationProperty::classIdentifier = "org.inviwo.LayerInformationProperty";
std::string LayerInformationProperty::getClassIdentifier() const { return classIdentifier; }

namespace {

auto props(LayerInformationProperty& prop) {
    return std::tie(prop.layerType, prop.dimensions, prop.format, prop.channels, prop.swizzleMask);
}

auto meta(LayerInformationProperty& prop) {
    return std::tie(prop.dataRange, prop.valueRange, prop.valueName, prop.valueUnit,
                    prop.interpolation, prop.axesNames, prop.axesUnits, prop.wrapping[0],
                    prop.wrapping[1]);
}

}  // namespace

LayerInformationProperty::LayerInformationProperty(std::string_view identifier,
                                                   std::string_view displayName,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : BoolCompositeProperty(
          identifier, displayName,
          "A CompositeProperty holding properties to show a information about an image layer"_help,
          false, invalidationLevel, semantics)
    , layerType("layerType", "Layer Type", "Type of the layer (Color, Depth, or Picking)"_help)
    , dimensions("dimensions", "Dimensions",
                 util::ordinalCount(size2_t{0})
                     .set(InvalidationLevel::Valid)
                     .set(PropertySemantics::Text)
                     .set("Layer dimensions (pixel)"_help))
    , format("format", "Format", "Underlying data format of the layer"_help, "")
    , channels("channels", "Channels", "Number of different channels in the layer"_help, 0,
               {0, ConstraintBehavior::Immutable},
               {std::numeric_limits<size_t>::max(), ConstraintBehavior::Immutable}, 1,
               InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , swizzleMask(
          "swizzleMask", "Swizzle Mask",
          "The swizzle mask is used when sampling the layer for example in OpenGL shaders"_help)
    , interpolation{"interpolation",
                    "Interpolation",
                    "Type of interpolation which is used when sampling the layer for example in OpenGL shaders"_help,
                    {InterpolationType::Linear, InterpolationType::Nearest},
                    0}
    , wrapping{{{"xWrappingX",
                 "X Wrapping",
                 {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror},
                 0},
                {"yWrappingX",
                 "Y Wrapping",
                 {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror},
                 0}}}
    , dataRange("dataRange", "Data Range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(), 0.0,
                0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , valueRange("valueRange", "Value Range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(),
                 0.0, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , valueName{"valueName", "Value name", ""}
    , valueUnit{"valueUnit", "Value unit", ""}
    , axesNames{"axesNames", "Axes Names"}
    , axesUnits{"axesUnits", "Axes Units"} {

    getBoolProperty()
        ->setDisplayName("Keep Changes")
        .setInvalidationLevel(InvalidationLevel::Valid)
        .visibilityDependsOn(*this, [](const auto& p) { return !p.getReadOnly(); })
        .setCurrentStateAsDefault();

    util::for_each_in_tuple(
        [&](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            addProperty(e);
        },
        props(*this));

    util::for_each_in_tuple([&](auto& p) { addProperty(p); }, meta(*this));
}

LayerInformationProperty::LayerInformationProperty(const LayerInformationProperty& rhs)
    : BoolCompositeProperty(rhs)
    , layerType(rhs.layerType)
    , dimensions(rhs.dimensions)
    , format(rhs.format)
    , channels(rhs.channels)
    , swizzleMask(rhs.swizzleMask)
    , interpolation(rhs.interpolation)
    , wrapping(rhs.wrapping)
    , dataRange(rhs.dataRange)
    , valueRange(rhs.valueRange)
    , valueName(rhs.valueName)
    , valueUnit(rhs.valueUnit)
    , axesNames(rhs.axesNames)
    , axesUnits(rhs.axesUnits) {

    util::for_each_in_tuple([&](auto& e) { addProperty(e); }, props(*this));
    util::for_each_in_tuple([&](auto& e) { addProperty(e); }, meta(*this));
}

LayerInformationProperty* LayerInformationProperty::clone() const {
    return new LayerInformationProperty(*this);
}

void LayerInformationProperty::updateForNewLayer(const Layer& layer,
                                                 util::OverwriteState overwrite) {
    layerType.set(toString(layer.getLayerType()));
    dimensions.set(layer.getDimensions());
    format.set(layer.getDataFormat()->getString());
    channels.set(layer.getDataFormat()->getComponents());
    swizzleMask.set(toString(layer.getSwizzleMask()));
    util::for_each_in_tuple([&](auto& e) { e.setCurrentStateAsDefault(); }, props(*this));

    overwrite = (overwrite == util::OverwriteState::Yes && !isChecked()) ? util::OverwriteState::Yes
                                                                         : util::OverwriteState::No;

    util::updateDefaultState(dataRange, layer.dataMap.dataRange, overwrite);
    util::updateDefaultState(valueRange, layer.dataMap.valueRange, overwrite);
    util::updateDefaultState(valueName, layer.dataMap.valueAxis.name, overwrite);
    util::updateDefaultState(valueUnit, fmt::to_string(layer.dataMap.valueAxis.unit), overwrite);
    util::updateDefaultState(interpolation, layer.getInterpolation(), overwrite);
    for (auto&& [prop, axis] : util::zip(axesNames.strings, layer.axes)) {
        util::updateDefaultState(prop, axis.name, overwrite);
    }
    for (auto&& [prop, axis] : util::zip(axesUnits.strings, layer.axes)) {
        util::updateDefaultState(prop, fmt::to_string(axis.unit), overwrite);
    }
    for (auto&& [prop, wrap] : util::zip(wrapping, layer.getWrapping())) {
        util::updateDefaultState(prop, wrap, overwrite);
    }
}

void LayerInformationProperty::updateLayer(Layer& layer) {
    layer.dataMap.dataRange = dataRange.get();
    layer.dataMap.valueRange = valueRange.get();
    layer.dataMap.valueAxis.name = valueName.get();
    layer.dataMap.valueAxis.unit = units::unit_from_string(valueUnit.get());
    layer.setInterpolation(interpolation.getSelectedValue());
    for (auto&& [prop, axis] : util::zip(axesNames.strings, layer.axes)) {
        axis.name = prop.get();
    }
    for (auto&& [prop, axis] : util::zip(axesUnits.strings, layer.axes)) {
        axis.unit = units::unit_from_string(prop.get());
    }
    layer.setWrapping({wrapping[0].getSelectedValue(), wrapping[1].getSelectedValue()});
}

}  // namespace inviwo
