/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/network/networklock.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

const std::string ColumnMetaDataProperty::classIdentifier = "org.inviwo.ColumnMetaDataProperty";
std::string ColumnMetaDataProperty::getClassIdentifier() const { return classIdentifier; }

ColumnMetaDataProperty::ColumnMetaDataProperty(std::string_view identifier,
                                               std::string_view displayName, dvec2 range,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : CompositeProperty(std::string(identifier), std::string(displayName), invalidationLevel,
                        semantics)
    , columnRange_("columnRange", "Column Range", range.x, range.y, -DataFloat64::max(),
                   DataFloat64::max(), 0.0, 0.0, InvalidationLevel::InvalidOutput,
                   PropertySemantics("Text"))
    , overrideRange_("overrideRange", "Override Range", false)
    , customRange_("customRange", "Custom Range", 0.0, 1.0, -DataFloat64::max(), DataFloat64::max(),
                   0.0, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , resetToData_("resetToData", "Reset to Data",
                   [&]() {
                       NetworkLock lock(this);
                       overrideRange_.setChecked(true);
                       customRange_.set(dataRange_);
                   })
    , dataRange_("dataRange", dvec2(0.0))
    , columnIndex_("columnIndex", 0u) {

    addProperties(columnRange_, overrideRange_);
    overrideRange_.addProperties(customRange_, resetToData_);

    setCollapsed(true);

    columnRange_.setReadOnly(true);
}

ColumnMetaDataProperty::ColumnMetaDataProperty(const ColumnMetaDataProperty& rhs)
    : CompositeProperty(rhs)
    , columnRange_(rhs.columnRange_)
    , overrideRange_(rhs.overrideRange_)
    , customRange_(rhs.customRange_)
    , resetToData_(rhs.resetToData_)
    , dataRange_(rhs.dataRange_)
    , columnIndex_(rhs.columnIndex_) {

    addProperties(columnRange_, overrideRange_);
    overrideRange_.addProperties(customRange_, resetToData_);
}

ColumnMetaDataProperty& ColumnMetaDataProperty::operator=(const ColumnMetaDataProperty& rhs) {
    if (this != &rhs) {
        columnRange_.set(rhs.columnRange_);
        overrideRange_.set(&rhs.overrideRange_);
        customRange_.set(rhs.customRange_);
        dataRange_ = rhs.dataRange_;
        columnIndex_ = rhs.columnIndex_;
    }
    return *this;
}

ColumnMetaDataProperty* ColumnMetaDataProperty::clone() const {
    return new ColumnMetaDataProperty(*this);
}

void ColumnMetaDataProperty::setRange(dvec2 columnRange, dvec2 dataRange) {
    columnRange_.set(columnRange);
    dataRange_ = dataRange;
}

dvec2 ColumnMetaDataProperty::getRange() const {
    if (overrideRange_.isChecked()) {
        return customRange_;
    } else {
        return columnRange_;
    }
}

void ColumnMetaDataProperty::setColumnIndex(size_t index) { columnIndex_ = index; }

size_t ColumnMetaDataProperty::getColumnIndex() const { return columnIndex_; }

void ColumnMetaDataProperty::serialize(Serializer& s) const {
    CompositeProperty::serialize(s);
    dataRange_.serialize(s, getSerializationMode());
    columnIndex_.serialize(s, getSerializationMode());
}

void ColumnMetaDataProperty::deserialize(Deserializer& d) {
    CompositeProperty::deserialize(d);
    dataRange_.deserialize(d, getSerializationMode());
    columnIndex_.deserialize(d, getSerializationMode());
}

}  // namespace inviwo
