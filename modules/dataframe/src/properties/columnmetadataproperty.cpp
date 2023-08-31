/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/dataframe/properties/columnmetadataproperty.h>

#include <inviwo/core/datastructures/unitsystem.h>         // for Unit
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/minmaxproperty.h>         // for DoubleMinMaxProperty
#include <inviwo/core/properties/property.h>               // for updateDefaultState, OverwriteS...
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/properties/stringproperty.h>         // for StringProperty
#include <inviwo/core/util/formats.h>                      // for DataFloat64, DataFormatBase
#include <inviwo/core/util/glmvec.h>                       // for dvec2
#include <inviwo/dataframe/datastructures/column.h>        // for Column, ColumnType, ColumnType...

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <type_traits>  // for remove_extent_t

#include <fmt/core.h>       // for basic_string_view
#include <fmt/format.h>     // for to_string
#include <glm/common.hpp>   // for max, min
#include <glm/vec2.hpp>     // for operator!=, operator==, vec<>:...
#include <llnl-units/units.hpp>  // for unit_from_string

namespace inviwo {

const std::string ColumnMetaDataProperty::classIdentifier = "org.inviwo.ColumnMetaDataProperty";
std::string ColumnMetaDataProperty::getClassIdentifier() const { return classIdentifier; }

ColumnMetaDataProperty::ColumnMetaDataProperty(std::string_view identifier,
                                               std::string_view displayName, dvec2 range,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, false, invalidationLevel, semantics)
    , header_{"header", "Header"}
    , type_{"type", "Type"}
    , range_("range", "Range", range.x, range.y, -DataFloat64::max(), DataFloat64::max(), 0.0, 0.0,
             InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , unit_{"unit", "Unit"}
    , drop_{"drop", "Drop", false} {

    getBoolProperty()
        ->setDisplayName("Keep Changes")
        .setInvalidationLevel(InvalidationLevel::Valid)
        .visibilityDependsOn(*this, [](const auto& p) { return !p.getReadOnly(); })
        .setCurrentStateAsDefault();

    type_.setReadOnly(true);

    addProperties(header_, type_, range_, unit_, drop_);
    setCollapsed(true).setCurrentStateAsDefault();
}

ColumnMetaDataProperty::ColumnMetaDataProperty(const ColumnMetaDataProperty& rhs)
    : BoolCompositeProperty(rhs)
    , header_{rhs.header_}
    , type_{rhs.type_}
    , range_{rhs.range_}
    , unit_{rhs.unit_}
    , drop_{rhs.drop_} {

    addProperties(header_, type_, range_, unit_, drop_);
}

ColumnMetaDataProperty& ColumnMetaDataProperty::operator=(const ColumnMetaDataProperty& rhs) {
    if (this != &rhs) {
        header_.set(rhs.header_.get());
        type_.set(rhs.type_.get());
        range_.set(rhs.range_.get());
        unit_.set(rhs.unit_.get());
        drop_.set(rhs.drop_.get());
    }
    return *this;
}

ColumnMetaDataProperty* ColumnMetaDataProperty::clone() const {
    return new ColumnMetaDataProperty(*this);
}

const std::string& ColumnMetaDataProperty::getHeader() const { return header_.get(); }
dvec2 ColumnMetaDataProperty::getRange() const { return range_.get(); }
Unit ColumnMetaDataProperty::getUnit() const { return units::unit_from_string(unit_.get()); }
bool ColumnMetaDataProperty::getDrop() const { return drop_.get(); }
std::string ColumnMetaDataProperty::getType() const { return type_.get(); }

void ColumnMetaDataProperty::updateForNewColumn(const Column& col, util::OverwriteState overwrite) {
    auto type = [](const Column& col) -> std::string {
        switch (col.getColumnType()) {
            case ColumnType::Categorical:
                return "Categorical";
            case ColumnType::Index:
                return "Index";
            case ColumnType::Ordinal:
            default:
                return col.getBuffer()->getDataFormat()->getString();
        }
    };

    overwrite = (overwrite == util::OverwriteState::Yes && !isChecked()) ? util::OverwriteState::Yes
                                                                         : util::OverwriteState::No;
    range_.setReadOnly(col.getColumnType() == ColumnType::Categorical ||
                       col.getColumnType() == ColumnType::Index);
    drop_.setReadOnly(col.getColumnType() == ColumnType::Index);

    setDisplayName(col.getHeader());
    util::updateDefaultState(header_, col.getHeader(), overwrite);
    util::updateDefaultState(type_, type(col), overwrite);
    util::updateDefaultState(range_, col.getRange(), overwrite);
    util::updateDefaultState(unit_, fmt::to_string(col.getUnit()), overwrite);
    util::updateDefaultState(drop_, false, overwrite);
}

void ColumnMetaDataProperty::updateColumn(Column& col) const {
    col.setHeader(getHeader());
    col.setCustomRange(getRange());
    col.setUnit(getUnit());
}

}  // namespace inviwo
