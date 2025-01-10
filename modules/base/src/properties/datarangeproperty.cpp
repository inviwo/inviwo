/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/base/properties/datarangeproperty.h>

#include <inviwo/core/datastructures/datamapper.h>         // for DataMapper
#include <inviwo/core/ports/volumeport.h>                  // for VolumeInport
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/minmaxproperty.h>         // for DoubleMinMaxProperty
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/properties/valuewrapper.h>           // for PropertySerializationMode, Pro...
#include <inviwo/core/util/formats.h>                      // for DataFloat64
#include <inviwo/core/util/glmvec.h>                       // for dvec2

#include <functional>   // for __base, function
#include <limits>       // for numeric_limits
#include <type_traits>  // for remove_extent_t

namespace inviwo {
class Volume;

std::string_view DataRangeProperty::getClassIdentifier() const { return classIdentifier; }

DataRangeProperty::DataRangeProperty(std::string_view identifier, std::string_view displayName,
                                     bool customRanges, InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, invalidationLevel, semantics}
    , customRanges_{customRanges}
    , dataRange_{"dataRange",
                 "Data range",
                 0.,
                 255.0,
                 -DataFloat64::max(),
                 DataFloat64::max(),
                 0.0,
                 0.0,
                 InvalidationLevel::InvalidOutput,
                 PropertySemantics::Text}
    , valueRange_{"valueRange",
                  "Value range",
                  0.,
                  255.0,
                  -DataFloat64::max(),
                  DataFloat64::max(),
                  0.0,
                  0.0,
                  InvalidationLevel::InvalidOutput,
                  PropertySemantics::Text}
    , useCustomRange_{"useCustomRange", "Use Custom Range", false}
    , customDataRange_{"customDataRange",
                       "Data Range",
                       0.0,
                       1.0,
                       std::numeric_limits<double>::lowest(),
                       std::numeric_limits<double>::max(),
                       0.01,
                       0.0,
                       InvalidationLevel::InvalidOutput,
                       PropertySemantics::Text}
    , customValueRange_{"customValueRange",
                        "Value Range",
                        0.0,
                        1.0,
                        std::numeric_limits<double>::lowest(),
                        std::numeric_limits<double>::max(),
                        0.01,
                        0.0,
                        InvalidationLevel::InvalidOutput,
                        PropertySemantics::Text}
    , copyFromInput_{"copyFromInput", "Copy Range from Input", [&]() {
                         customDataRange_.set(dataRange_);
                         customValueRange_.set(valueRange_);
                     }} {

    dataRange_.setReadOnly(true);
    valueRange_.setReadOnly(true);
    dataRange_.setSerializationMode(PropertySerializationMode::All);
    valueRange_.setSerializationMode(PropertySerializationMode::All);

    useCustomRange_.addProperties(customDataRange_, customValueRange_, copyFromInput_);
    useCustomRange_.setVisible(customRanges_);
    useCustomRange_.setCollapsed(true);
    addProperties(dataRange_, valueRange_, useCustomRange_);
}

DataRangeProperty::DataRangeProperty(std::string_view identifier, std::string_view displayName,
                                     VolumeInport& port, bool customRanges,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : DataRangeProperty{identifier, displayName, customRanges, invalidationLevel, semantics} {

    port.onChange([&]() {
        if (port.hasData()) {
            const auto data = port.getData();
            dataRange_.set(data->dataMap.dataRange);
            valueRange_.set(data->dataMap.valueRange);
        }
    });
}

DataRangeProperty::DataRangeProperty(std::string_view identifier, std::string_view displayName,
                                     LayerInport& port, bool customRanges,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : DataRangeProperty{identifier, displayName, customRanges, invalidationLevel, semantics} {

    port.onChange([&]() {
        if (port.hasData()) {
            const auto data = port.getData();
            dataRange_.set(data->dataMap.dataRange);
            valueRange_.set(data->dataMap.valueRange);
        }
    });
}

DataRangeProperty::DataRangeProperty(const DataRangeProperty& rhs)
    : CompositeProperty{rhs}
    , customRanges_{rhs.customRanges_}
    , dataRange_{rhs.dataRange_}
    , valueRange_{rhs.valueRange_}
    , useCustomRange_{rhs.useCustomRange_}
    , customDataRange_{rhs.customDataRange_}
    , customValueRange_{rhs.customValueRange_}
    , copyFromInput_{rhs.copyFromInput_} {
    useCustomRange_.addProperties(customDataRange_, customValueRange_, copyFromInput_);
    useCustomRange_.setVisible(customRanges_);
    addProperties(dataRange_, valueRange_, useCustomRange_);
}

DataRangeProperty* DataRangeProperty::clone() const { return new DataRangeProperty{*this}; }

void DataRangeProperty::updateFromVolume(std::shared_ptr<Volume> volume) {
    if (!volume) return;
    dataRange_.set(volume->dataMap.dataRange);
    valueRange_.set(volume->dataMap.valueRange);
}

void DataRangeProperty::updateFromLayer(std::shared_ptr<Layer> layer) {
    if (!layer) return;
    dataRange_.set(layer->dataMap.dataRange);
    valueRange_.set(layer->dataMap.valueRange);
}

void DataRangeProperty::setDataRange(const dvec2& range) { dataRange_.set(range); }

void DataRangeProperty::setValueRange(const dvec2& range) { valueRange_.set(range); }

dvec2 DataRangeProperty::getDataRange() const {
    if (getCustomRangeEnabled()) {
        return customDataRange_;
    } else {
        return dataRange_;
    }
}

dvec2 DataRangeProperty::getCustomDataRange() const { return customDataRange_.get(); }

dvec2 DataRangeProperty::getValueRange() const {
    if (getCustomRangeEnabled()) {
        return customValueRange_;
    } else {
        return valueRange_;
    }
}

dvec2 DataRangeProperty::getCustomValueRange() const { return customValueRange_.get(); }

bool DataRangeProperty::getCustomRangeEnabled() const {
    return (customRanges_ && useCustomRange_.isChecked());
}

}  // namespace inviwo
