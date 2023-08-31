/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2023 Inviwo Foundation
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

#include <modules/base/properties/volumeinformationproperty.h>

#include <inviwo/core/algorithm/markdown.h>                // for operator""_help
#include <inviwo/core/datastructures/datamapper.h>         // for DataMapper
#include <inviwo/core/datastructures/histogram.h>          // for HistogramContainer
#include <inviwo/core/datastructures/unitsystem.h>         // for Axis, Unit
#include <inviwo/core/datastructures/volume/volume.h>      // for Volume
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/minmaxproperty.h>         // for DoubleMinMaxProperty, MinMaxPr...
#include <inviwo/core/properties/ordinalproperty.h>        // for OrdinalProperty, ordinalCount
#include <inviwo/core/properties/property.h>               // for updateDefaultState, OverwriteS...
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/properties/stringproperty.h>         // for StringProperty
#include <inviwo/core/properties/stringsproperty.h>        // for StringsProperty
#include <inviwo/core/properties/templateproperty.h>       // for TemplateProperty
#include <inviwo/core/properties/valuewrapper.h>           // for PropertySerializationMode, Pro...
#include <inviwo/core/util/foreacharg.h>                   // for for_each_in_tuple
#include <inviwo/core/util/formats.h>                      // for DataFloat64, DataFormatBase
#include <inviwo/core/util/glmvec.h>                       // for dvec2, size3_t
#include <inviwo/core/util/zip.h>                          // for zip, zipIterator, zipper

#include <cstddef>     // for size_t
#include <functional>  // for __base
#include <tuple>       // for tuple, tie

#include <fmt/core.h>       // for basic_string_view
#include <fmt/format.h>     // for to_string
#include <glm/vec2.hpp>     // for operator!=, operator==, vec
#include <glm/vec3.hpp>     // for vec, vec<>::(anonymous)
#include <llnl-units/units.hpp>  // for unit_from_string

namespace inviwo {

const std::string VolumeInformationProperty::classIdentifier =
    "org.inviwo.VolumeInformationProperty";
std::string VolumeInformationProperty::getClassIdentifier() const { return classIdentifier; }

namespace {
auto props(VolumeInformationProperty& prop) {
    return std::tie(prop.dimensions, prop.format, prop.channels, prop.numVoxels);
}

auto meta(VolumeInformationProperty& prop) {
    return std::tie(prop.dataRange, prop.valueRange, prop.valueName, prop.valueUnit,
                    prop.interpolation, prop.axesNames, prop.axesUnits, prop.wrapping[0],
                    prop.wrapping[1], prop.wrapping[2]);
}

}  // namespace
VolumeInformationProperty::VolumeInformationProperty(std::string_view identifier,
                                                     std::string_view displayName,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName,
                            "Various information and statistics about a volume"_help, false,
                            invalidationLevel, semantics)
    , dimensions{"dimensions", "Dimensions",
                 util::ordinalCount(size3_t{0})
                     .set(InvalidationLevel::Valid)
                     .set(PropertySemantics::Text)}
    , format{"format", "Format", ""}
    , channels{"channels", "Channels",
               util::ordinalCount(size_t{0})
                   .set(InvalidationLevel::Valid)
                   .set(PropertySemantics::Text)}
    , numVoxels{"numVoxels", "Number of Voxels",
                util::ordinalCount(size_t{0})
                    .set(InvalidationLevel::Valid)
                    .set(PropertySemantics::Text)}
    , dataRange{"dataRange",
                "Data range",
                0.,
                255.0,
                -DataFloat64::max(),
                DataFloat64::max(),
                0.0,
                0.0,
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Text}
    , valueRange{"valueRange",
                 "Value range",
                 0.,
                 255.0,
                 -DataFloat64::max(),
                 DataFloat64::max(),
                 0.0,
                 0.0,
                 InvalidationLevel::InvalidOutput,
                 PropertySemantics::Text}
    , valueName{"valueName", "Value name", ""}
    , valueUnit{"valueUnit", "Value unit", ""}
    , interpolation{"interpolation",
                    "Interpolation",
                    {InterpolationType::Linear, InterpolationType::Nearest},
                    0}
    , axesNames{"axesNames", "Axes Names"}
    , axesUnits{"axesUnits", "Axes Units"}
    , wrapping{{
          {"xWrappingX", "X Wrapping", {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}, 0},
          {"yWrappingX", "Y Wrapping", {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}, 0},
          {"zWrappingX", "Z Wrapping", {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}, 0},
      }} {

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

VolumeInformationProperty::VolumeInformationProperty(const VolumeInformationProperty& rhs)
    : BoolCompositeProperty{rhs}
    , dimensions{rhs.dimensions}
    , format{rhs.format}
    , channels{rhs.channels}
    , numVoxels{rhs.numVoxels}
    , dataRange{rhs.dataRange}
    , valueRange{rhs.valueRange}
    , valueName{rhs.valueName}
    , valueUnit{rhs.valueUnit}
    , interpolation{rhs.interpolation}
    , axesNames{rhs.axesNames}
    , axesUnits{rhs.axesUnits}
    , wrapping{rhs.wrapping} {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props(*this));
    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, meta(*this));
}

VolumeInformationProperty* VolumeInformationProperty::clone() const {
    return new VolumeInformationProperty(*this);
}

void VolumeInformationProperty::updateForNewVolume(const Volume& volume,
                                                   util::OverwriteState overwrite) {
    const auto dim = volume.getDimensions();
    dimensions.set(dim);
    format.set(volume.getDataFormat()->getString());
    channels.set(volume.getDataFormat()->getComponents());
    numVoxels.set(dim.x * dim.y * dim.z);
    util::for_each_in_tuple([&](auto& e) { e.setCurrentStateAsDefault(); }, props(*this));

    overwrite = (overwrite == util::OverwriteState::Yes && !isChecked()) ? util::OverwriteState::Yes
                                                                         : util::OverwriteState::No;

    util::updateDefaultState(dataRange, volume.dataMap_.dataRange, overwrite);
    util::updateDefaultState(valueRange, volume.dataMap_.valueRange, overwrite);
    util::updateDefaultState(valueName, volume.dataMap_.valueAxis.name, overwrite);
    util::updateDefaultState(valueUnit, fmt::to_string(volume.dataMap_.valueAxis.unit), overwrite);
    util::updateDefaultState(interpolation, volume.getInterpolation(), overwrite);
    for (auto&& [prop, axis] : util::zip(axesNames.strings, volume.axes)) {
        util::updateDefaultState(prop, axis.name, overwrite);
    }
    for (auto&& [prop, axis] : util::zip(axesUnits.strings, volume.axes)) {
        util::updateDefaultState(prop, fmt::to_string(axis.unit), overwrite);
    }
    for (auto&& [prop, wrap] : util::zip(wrapping, volume.getWrapping())) {
        util::updateDefaultState(prop, wrap, overwrite);
    }
}

void VolumeInformationProperty::updateVolume(Volume& volume) {
    if (volume.dataMap_.dataRange != dataRange.get()) {
        if (volume.hasHistograms()) {
            volume.getHistograms().clear();
        }
    }

    volume.dataMap_.dataRange = dataRange.get();
    volume.dataMap_.valueRange = valueRange.get();
    volume.dataMap_.valueAxis.name = valueName.get();
    volume.dataMap_.valueAxis.unit = units::unit_from_string(valueUnit.get());
    volume.setInterpolation(interpolation.getSelectedValue());
    for (auto&& [prop, axis] : util::zip(axesNames.strings, volume.axes)) {
        axis.name = prop.get();
    }
    for (auto&& [prop, axis] : util::zip(axesUnits.strings, volume.axes)) {
        axis.unit = units::unit_from_string(prop.get());
    }
    volume.setWrapping({wrapping[0].getSelectedValue(), wrapping[1].getSelectedValue(),
                        wrapping[2].getSelectedValue()});
}

}  // namespace inviwo
