/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/properties/valueaxisproperty.h>

#include <fmt/format.h>

namespace inviwo {

std::string_view ValueAxisProperty::getClassIdentifier() const { return classIdentifier; }

ValueAxisProperty::ValueAxisProperty(std::string_view identifier, std::string_view displayName,
                                     bool customAxis, InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, invalidationLevel, semantics}
    , customAxis_{customAxis}
    , valueName_{"valueName", "Value name", ""}
    , valueUnit_{"valueUnit", "Value unit", ""}
    , useCustomAxis_{"useCustomAxis", "Use Custom Value Axis", false}
    , customValueName_{"customValueName", "Value name", ""}
    , customValueUnit_{"customValueUnit", "Value unit", ""}
    , copyFromInput_{"copyFromInput", "Copy Range from Input", [&]() {
                         customValueName_.set(&valueName_);
                         customValueUnit_.set(&valueUnit_);
                     }} {

    valueName_.setReadOnly(true);
    valueUnit_.setReadOnly(true);
    valueName_.setSerializationMode(PropertySerializationMode::All);
    valueUnit_.setSerializationMode(PropertySerializationMode::All);

    useCustomAxis_.addProperties(customValueName_, customValueUnit_, copyFromInput_);
    useCustomAxis_.setVisible(customAxis_);
    useCustomAxis_.setCollapsed(true);
    addProperties(valueName_, valueUnit_, useCustomAxis_);
}

ValueAxisProperty::ValueAxisProperty(std::string_view identifier, std::string_view displayName,
                                     VolumeInport& port, bool customAxis,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : ValueAxisProperty{identifier, displayName, customAxis, invalidationLevel, semantics} {

    port.onChange([&]() {
        if (port.hasData()) {
            const auto data = port.getData();
            valueName_.set(data->dataMap.valueAxis.name);
            valueUnit_.set(fmt::to_string(data->dataMap.valueAxis.unit));
        }
    });
}

ValueAxisProperty::ValueAxisProperty(std::string_view identifier, std::string_view displayName,
                                     LayerInport& port, bool customAxis,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : ValueAxisProperty{identifier, displayName, customAxis, invalidationLevel, semantics} {

    port.onChange([&]() {
        if (port.hasData()) {
            const auto data = port.getData();
            valueName_.set(data->dataMap.valueAxis.name);
            valueUnit_.set(fmt::to_string(data->dataMap.valueAxis.unit));
        }
    });
}

ValueAxisProperty::ValueAxisProperty(const ValueAxisProperty& rhs)
    : CompositeProperty{rhs}
    , customAxis_{rhs.customAxis_}
    , valueName_{rhs.valueName_}
    , valueUnit_{rhs.valueUnit_}
    , useCustomAxis_{rhs.useCustomAxis_}
    , customValueName_{rhs.customValueName_}
    , customValueUnit_{rhs.customValueUnit_}
    , copyFromInput_{rhs.copyFromInput_} {
    useCustomAxis_.addProperties(customValueName_, customValueUnit_, copyFromInput_);
    useCustomAxis_.setVisible(customAxis_);
    addProperties(valueName_, valueUnit_, useCustomAxis_);
}

ValueAxisProperty* ValueAxisProperty::clone() const { return new ValueAxisProperty{*this}; }

void ValueAxisProperty::updateFromVolume(std::shared_ptr<Volume> volume) {
    if (!volume) return;
    valueName_.set(volume->dataMap.valueAxis.name);
    valueUnit_.set(fmt::to_string(volume->dataMap.valueAxis.unit));
}

void ValueAxisProperty::updateFromLayer(std::shared_ptr<Layer> layer) {
    if (!layer) return;
    valueName_.set(layer->dataMap.valueAxis.name);
    valueUnit_.set(fmt::to_string(layer->dataMap.valueAxis.unit));
}

void ValueAxisProperty::setValueName(std::string_view name) { valueName_.set(name); }

void ValueAxisProperty::setValueUnit(Unit unit) { valueUnit_.set(fmt::to_string(unit)); }

std::string_view ValueAxisProperty::getValueName() const {
    if (getCustomAxisEnabled()) {
        return customValueName_;
    } else {
        return valueName_;
    }
}

std::string_view ValueAxisProperty::getCustomValueName() const { return customValueName_.get(); }

Unit ValueAxisProperty::getValueUnit() const {
    if (getCustomAxisEnabled()) {
        return units::unit_from_string(customValueUnit_);
    } else {
        return units::unit_from_string(valueUnit_);
    }
}

Unit ValueAxisProperty::getCustomValueUnit() const {
    return units::unit_from_string(customValueUnit_);
}

bool ValueAxisProperty::getCustomAxisEnabled() const {
    return (customAxis_ && useCustomAxis_.isChecked());
}

}  // namespace inviwo
